#!/usr/bin/perl
# Contest UI for the MO Submitter
# (c) 2007 Martin Mares <mj@ucw.cz>

use strict;
use warnings;

BEGIN {
	defined $ENV{"MO_ROOT"} or die "Please set MO_ROOT to the contest root directory first.\n";
}
use lib $ENV{"MO_ROOT"} . "/lib/perl5";
use lib $ENV{"MO_ROOT"} . "/submit/lib/perl5";

use MO::Submit;
use Sherlock::Object;
use POSIX;
use IO::File;
use File::stat;
use Gtk2 -init;

my $conn = new MO::Submit;

### GUI INITIALIZATION ###

sub init_refresh();
sub start_refresh_timer($);
sub stop_refresh_timer();
my $force_refresh = 0;
sub refresh();

sub submit($);

my $busy_cursor = Gtk2::Gdk::Cursor->new('watch');

# The main window
my $window = Gtk2::Window->new('toplevel');
$window->signal_connect("delete-event" => sub { Gtk2->main_quit });
$window->set_title($conn->{"Contest"} . " Submitter");
$window->set_wmclass("submitter", "Submitter");
$window->set_default_size(640, 480);

# The title label
my $title_lab = Gtk2::Label->new;
$title_lab->set_markup("<span size='x-large'>" . $conn->{"Contest"} . "</span>");

# The row of buttons
my $b_submit = Gtk2::Button->new('Submit');
$b_submit->set_image(Gtk2::Image->new_from_stock('gtk-apply', 'button'));
$b_submit->signal_connect(clicked => sub { submit(0) });
$b_submit->set_sensitive(0);

my $b_check = Gtk2::Button->new('Check');
$b_check->set_image(Gtk2::Image->new_from_stock('gtk-find', 'button'));
$b_check->signal_connect(clicked => sub { submit(1) });
$b_check->set_sensitive(0);

my $b_refresh = Gtk2::Button->new('Refresh');
$b_refresh->set_image(Gtk2::Image->new_from_stock('gtk-refresh', 'button'));
$b_refresh->signal_connect(clicked => sub { start_refresh_timer(1) });

my $button_box = Gtk2::HBox->new;
$button_box->pack_start_defaults($b_submit);
$button_box->pack_start_defaults($b_check);
$button_box->pack_start_defaults($b_refresh);
$button_box->set_border_width(5);

# The list of tasks
my $task_store = Gtk2::ListStore->new('Glib::String', 'Glib::String');

my $task_view = Gtk2::TreeView->new($task_store);
my $task_renderer = Gtk2::CellRendererText->new;
my $task_col1 = Gtk2::TreeViewColumn->new_with_attributes("Task", $task_renderer, "text", 0);
$task_view->append_column($task_col1);
my $task_col2 = Gtk2::TreeViewColumn->new_with_attributes("Status", $task_renderer, "text", 1);
$task_view->append_column($task_col2);
$task_view->set_headers_visible(0);

my $task_scroll = Gtk2::ScrolledWindow->new;
$task_scroll->set_policy("automatic", "automatic");
$task_scroll->add($task_view);
$task_scroll->set_border_width(5);

my $task_frame = Gtk2::Frame->new("Tasks");
$task_frame->add($task_scroll);

my $selected_task;
my $task_sel = $task_view->get_selection;
$task_sel->set_mode('single');
$task_sel->signal_connect(changed => sub {
	my $iter = $_[0]->get_selected;
	if ($iter) {
		$selected_task = $task_store->get($iter, 0);
		$b_submit->set_sensitive(1);
		$b_check->set_sensitive(1);
	} else {
		$selected_task = undef;
		print "Deselected task\n";
		$b_submit->set_sensitive(0);
		$b_check->set_sensitive(0);
	}
});

my $status_bar = Gtk2::Statusbar->new;
my $bar_ctx = $status_bar->get_context_id('xyzzy');

my $vbox = Gtk2::VBox->new;
$vbox->pack_start($title_lab, 0, 0, 10);
$vbox->pack_start($task_frame, 1, 1, 0);
$vbox->pack_start($button_box, 0, 0, 0);
$vbox->pack_start($status_bar, 0, 0, 0);

$window->add($vbox);
$window->signal_connect("expose-event" => sub { init_refresh(); return 0; });
$window->show_all;

### REFRESHING ###

my $last_status_id;

sub msg($) {
	print "GUI: ", $_[0], "\n" if $conn->{"Trace"};
}

sub status($) {
	msg($_[0]);
	defined $last_status_id and $status_bar->remove($bar_ctx, $last_status_id);
	$last_status_id = $status_bar->push($bar_ctx, shift @_);
}

sub busy($) {
	status($_[0]);
	$window->window->set_cursor($busy_cursor);
	$window->Gtk2::Gdk::flush;
}

sub ready($) {
	status($_[0]);
	$window->window->set_cursor(undef);
	$window->Gtk2::Gdk::flush;
}

my $window_inited = 0;
sub init_refresh()
{
	if (!$window_inited) {
		$force_refresh = 1;
		start_refresh_timer(1);
		$window_inited = 1;
	}
	return 1;
}

my $refresh_timer_id;

sub timed_refresh()
{
	refresh();
	return 1;	# We wish to re-run the timer
}

sub start_refresh_timer($) {
	my ($go) = @_;
	stop_refresh_timer();
	refresh() if $go;
	$refresh_timer_id = Glib::Timeout->add($conn->{"RefreshTimer"}, \&timed_refresh);
}

sub stop_refresh_timer() {
	if (defined $refresh_timer_id) {
		Glib::Source->remove($refresh_timer_id);
		$refresh_timer_id = undef;
	}
}

my $task_status_object;
my @task_parts = ();
my @task_stat = ();

sub recalc_task_list() {
	my @new_tp = ();
	my @new_stat = ();
	foreach my $t ($task_status_object->getarray("(T")) {
		my $task = $t->get("T");
		foreach my $p ($t->getarray("(P")) {
			my $part = $p->get("P");
			my $taskpart = ($task eq $part) ? $task : "$task/$part";
			push @new_tp, $taskpart;
			my $status = "---";
			my $current_ver = $p->get("V");
			foreach my $v ($p->getarray("(V")) {
				if ($v->get("V") == $current_ver) {
					my $time = strftime("%H:%M:%S", localtime $v->get("T"));
					$status = "OK (" .
						"$part." . $v->get("X") . ", " .
						$v->get("L") . " bytes, " .
						$v->get("S") . " $time)";
					last;
				}
			}
			push @new_stat, $status;
		}
	}

	if (join("\n", @new_tp) ne join("\n", @task_parts)) {
		# The tasks have changed, repopulate the whole structure
		$task_store->clear;
		my @s = @new_stat;
		foreach my $taskpart (@new_tp) {
			my $iter = $task_store->append;
			$task_store->set($iter,
				0, $taskpart,
				1, shift @s);
		}
	} else {
		# Update the task status
		my @s = @task_stat;
		my @ns = @new_stat;
		$task_store->foreach(sub {
			my ($obj, $path, $iter) = @_;
			if ($s[0] ne $ns[0]) {
				$task_store->set($iter, 1, $ns[0]);
			}
			shift @s;
			shift @ns;
			return 0;
		});
	}

	@task_parts = @new_tp;
	@task_stat = @new_stat;
}

sub refresh()
{
	if (!$conn->is_connected || $force_refresh) {
		busy("Connecting to server...");
		if ($conn->connect) {
			ready("Connected successfully");
		} else {
			ready($conn->{"error"});
		}
	}
	if ($conn->is_connected) {
		busy("Updating status...");
		my $r = new Sherlock::Object("!" => "STATUS");
		$r = $conn->request($r);
		if (!defined $r) {
			ready($conn->{"error"});
		} elsif ($r->get("-")) {
			ready($r->get("-"));
		} else {
			$task_status_object = $r;
			recalc_task_list;
			$force_refresh = 0;
			ready("Ready");
		}
	}
	if (!$conn->is_connected && !$force_refresh) {
		# Retry
		$conn->log("Retrying");
		$force_refresh = 1;
		refresh();
	}
}

### SUBMITTING ###

my $subwin;
my $chooser;
my $subwin_vbox;
my $subwin_label;
my $bbutton_box;
my $submitting_label;
my $text_frame;
my $status_label;
my $check_only;
my $submit_filename;
my $submit_extension;
my %submit_fn_cache = ();

sub end_submit($) {
	my ($close) = @_;
	$subwin->destroy if $close;
	start_refresh_timer(1);
}

sub finish_submit() {
	my $button = Gtk2::Button->new_from_stock('gtk-close');
	$button->signal_connect(clicked => sub { end_submit(1) });

	$bbutton_box = Gtk2::HButtonBox->new;
	$bbutton_box->pack_start_defaults($button);
	$subwin_vbox->pack_start($bbutton_box, 0, 0, 10);

	ready("Ready");
	$subwin->show_all;
	$subwin->window->set_cursor(undef);
}

sub submit_ok() {
	$status_label->set_markup("<span size='large'>Submitted OK</span>");
	$submitting_label->set_markup("<span size='large'>The task has been successfully submitted.</span>");
	refresh();
	finish_submit();
}

sub submit_failed($) {
	my ($msg) = @_;
	$status_label->set_markup("<span size='large'>Submit failed</span>");
	$submitting_label->set_markup("<span size='large'>$msg</span>");
	finish_submit();
}

sub run_submit() {
	my ($task, $part) = split /\//, $selected_task;
	defined $part or $part = $task;

	if (defined $conn->{"History"}) {
		busy("Submitting locally to " . $conn->{"History"});
		my $err = $conn->write_history($task, $part, $submit_extension, $submit_filename);
		if (defined $err) {
			submit_failed("Recording to local history failed\n($err)");
			return;
		}
	}

	if ($conn->is_connected) {
		busy("Checking server status...");
		my $r = new Sherlock::Object("!" => "NOP");
		$r = $conn->request($r);
	}
	if (!$conn->is_connected) {
		busy("Reconnecting to server...");
		if (!$conn->connect) {
			ready($conn->{"error"});
			submit_failed("Unable to connect to the server");
			return;
		}
	}
	busy("Submitting...");

	my $fh = new IO::File($submit_filename);
	if (!$fh) {
		submit_failed("Unable to open $submit_filename\n($!)");
		return;
	}
	my $stat = File::stat::populate($fh->stat);
	if (!$stat) {
		submit_failed("Unable to stat $submit_filename\n($!)");
		return;
	}
	my $size = $stat->size;

	my $r = new Sherlock::Object("!" => "SUBMIT", "T" => $task, "P" => $part, "X" => $submit_extension, "S" => $size);
	$r = $conn->request($r);
	if (!defined($r)) {
		submit_failed("Connection to the server lost");
		return;
	} elsif ($r->get("-")) {
		submit_failed($r->get("-"));
		return;
	}

	$r = $conn->send_file($fh, $size);
	if (!defined($r)) {
		submit_failed("Connection to the server lost");
		return;
	} elsif ($r->get("-")) {
		submit_failed($r->get("-"));
		return;
	}

	close $fh;
	submit_ok();
}

sub checks_ok() {
	if ($check_only) {
		$status_label->set_markup("<span size='large'>Checked successfully</span>");
		$submitting_label->set_markup("<span size='large'>The task has passed the checks.</span>");
		finish_submit();
		return;
	}

	$status_label->set_markup("<span size='large'>Submitting</span>");
	$subwin->show_all;

	# Continue when everything is displayed
	Glib::Idle->add(sub {
		$window->Gtk2::Gdk::flush;
		run_submit();
		return 0;
	});
}

sub checks_override() {
	$submitting_label = Gtk2::Label->new("Please wait...");
	$subwin_vbox->pack_start_defaults($submitting_label);

	$subwin->window->set_cursor($busy_cursor);
	$bbutton_box->destroy;
	$text_frame->destroy;
	checks_ok();
}

sub checks_failed($) {
	my ($msg) = @_;

	$status_label->set_markup("<span size='large'>Check failed</span>");
	$submitting_label->destroy;

	my $text_buffer = Gtk2::TextBuffer->new;
	$text_buffer->set_text($msg);

	my $text_view = Gtk2::TextView->new_with_buffer($text_buffer);
	$text_view->set_editable(0);
	$text_view->set_cursor_visible(0);

	my $text_scroll = Gtk2::ScrolledWindow->new;
	$text_scroll->set_policy("automatic", "automatic");
	$text_scroll->add($text_view);

	$text_frame = Gtk2::Frame->new("Checker log");
	$text_frame->add($text_scroll);

	$subwin_vbox->pack_start_defaults($text_frame);

	if ($check_only || !$conn->{"AllowOverride"}) {
		finish_submit();
		return;
	}

	my $close_button = Gtk2::Button->new_from_stock('gtk-close');
	$close_button->signal_connect(clicked => sub { end_submit(1) });

	my $anyway_button = Gtk2::Button->new('Submit anyway');
	$anyway_button->signal_connect(clicked => \&checks_override);

	$bbutton_box = Gtk2::HButtonBox->new;
	$bbutton_box->pack_start_defaults($anyway_button);
	$bbutton_box->pack_start_defaults($close_button);
	$bbutton_box->set_border_width(5);
	$subwin_vbox->pack_start($bbutton_box, 0, 0, 10);

	ready("Ready");
	$subwin->show_all;
	$subwin->window->set_cursor(undef);
}

sub run_checks() {
	($submit_extension) = ($submit_filename =~ /\.([^.]+)$/);
	if (!$submit_extension) {
		checks_failed("The filename does not have a valid extension");
		return;
	}
	if (!$conn->{"Checks"}) {
		checks_ok();
		return;
	}
	my $root = $conn->{"root"};
	my ($task, $part) = split /\//, $selected_task;
	defined $part or $part = "";
	my $verdict = `$root/bin/check -s "$submit_filename" $task $part 2>&1`;
	if ($?) {
		checks_failed($verdict);
	} else {
		checks_ok();
	}
}

sub do_submit() {
	$submit_filename = $chooser->get_filename;
	$submit_fn_cache{$selected_task} = $submit_filename;
	msg "Selected $submit_filename";
	defined $submit_filename or return;
	-f $submit_filename or return;

	$chooser->destroy;
	$bbutton_box->destroy;

	$status_label->set_markup("<span size='large'>Checking</span>");

	$submitting_label = Gtk2::Label->new("Please wait...");
	$subwin_vbox->pack_start_defaults($submitting_label);
	$subwin->show_all;
	$subwin->window->set_cursor($busy_cursor);

	# Continue when everything is displayed
	Glib::Idle->add(sub {
		$window->Gtk2::Gdk::flush;
		run_checks();
		return 0;
	});
}

sub submit($) {
	$check_only = shift @_;

	stop_refresh_timer();

	$subwin = Gtk2::Window->new('toplevel');
	$subwin->set_default_size(640, 480);
	$subwin->set_modal(1);
	$subwin->set_transient_for($window);
	$subwin->set_destroy_with_parent(1);
	$subwin->set_title("Submit task $selected_task");
	$subwin->set_wmclass("submitter", "Submitter");
	$subwin->signal_connect("delete-event" => sub { end_submit(0); return 0; });

	my $bb_submit = Gtk2::Button->new($check_only ? 'Check' : 'Submit');
	$bb_submit->set_image(Gtk2::Image->new_from_stock('gtk-apply', 'button'));
	$bb_submit->signal_connect(clicked => \&do_submit);

	my $bb_cancel = Gtk2::Button->new_from_stock('gtk-cancel');
	$bb_cancel->signal_connect(clicked => sub { end_submit(1) });

	$bbutton_box = Gtk2::HButtonBox->new;
	$bbutton_box->pack_start_defaults($bb_submit);
	$bbutton_box->pack_start_defaults($bb_cancel);
	$bbutton_box->set_border_width(5);

	my $subwin_label = Gtk2::Label->new;
	$subwin_label->set_markup("<span size='x-large'>" . ($check_only ? "Checking" : "Submitting") . " $selected_task</span>");

	$status_label = Gtk2::Label->new;
	$status_label->set_markup("<span size='large'>Please select file to " . ($check_only ? "check" : "submit") . "</span>");

	$chooser = Gtk2::FileChooserWidget->new("open");
	$chooser->set_local_only(1);
	$chooser->signal_connect("file-activated" => \&do_submit);
	$chooser->set_filename($submit_fn_cache{$selected_task}) if defined $submit_fn_cache{$selected_task};

	$subwin_vbox = Gtk2::VBox->new;
	$subwin_vbox->pack_start($subwin_label, 0, 0, 10);
	$subwin_vbox->pack_start($status_label, 0, 0, 10);
	$subwin_vbox->pack_start_defaults($chooser);
	$subwin_vbox->pack_start($bbutton_box, 0, 0, 0);

	$subwin->add($subwin_vbox);
	$subwin->show_all;
}

### MAIN ###

Gtk2->main;
exit 0;
