#!/usr/bin/perl

use strict;
use warnings;

use Gtk2 -init;

sub timer {
	print STDERR "Brum!\n";
	return 1;
}
Glib::Timeout->add(5000, \&timer);

my $cursor = Gtk2::Gdk::Cursor->new('watch');

my $window = Gtk2::Window->new ('toplevel');

my $b1 = Gtk2::Button->new ('Quit');
$b1->signal_connect (clicked => sub { Gtk2->main_quit });
my $b2 = Gtk2::Button->new ('Exit');
$b2->signal_connect (clicked => sub { Gtk2->main_quit });
my $b3 = Gtk2::Button->new ('Apage!');
$b3->signal_connect (clicked => sub { Gtk2->main_quit });
my $box = Gtk2::HBox->new();
$box->pack_start_defaults($b1);
$box->pack_start_defaults($b2);
$box->pack_start_defaults($b3);
$box->set_border_width(5);

my $bb = Gtk2::Button->new ('Brum!');
$bb->signal_connect (clicked => sub {
		my $dialog = Gtk2::MessageDialog->new($window, [qw/modal destroy-with-parent/], 'question', 'ok-cancel', "So what?");
		$dialog->set_default_response("ok");
		$dialog->signal_connect (response => sub { $_[0]->destroy });
		$dialog->show_all;
	});
#$bb->set_sensitive(0);

my $store = Gtk2::ListStore->new('Glib::Uint', 'Glib::String');
for (my $i=0; $i<10; $i++) {
	my $iter = $store->append;
	$store->set($iter, 0, $i, 1, "Hey ($i)");
}

my $tree = Gtk2::TreeView->new($store);
my $rend = Gtk2::CellRendererText->new;
my $col = Gtk2::TreeViewColumn->new_with_attributes("Int", $rend, "text", 0);
$tree->append_column($col);
$col = Gtk2::TreeViewColumn->new_with_attributes("String", $rend, "text", 1);
$tree->append_column($col);
$tree->set_headers_visible(0);

my $lay = Gtk2::ScrolledWindow->new;
$lay->set_policy("automatic", "automatic");
$lay->add($tree);
$lay->set_border_width(5);

my $frame = Gtk2::Frame->new("A list");
$frame->add($lay);

my $sel = $tree->get_selection;
$sel->set_mode('single');
$sel->signal_connect(changed => sub {
	my $iter = $_[0]->get_selected;
	my $val = $store->get($iter, 0);
	print "Selected $val\n";
	});

my $lab = Gtk2::Label->new;
$lab->set_markup("<span size='x-large'>Welcome to the Cave</span>");

my $bar = Gtk2::Statusbar->new;
my $barctx = $bar->get_context_id('xyzzy');
$bar->push($barctx, "Ready");

my $bbox = Gtk2::VBox->new();
$bbox->pack_start($lab, 0, 0, 0);
$bbox->pack_start($box, 0, 0, 0);
$bbox->pack_start($frame, 1, 1, 0);
$bbox->pack_start($bb, 0, 0, 0);
$bbox->pack_start($bar, 0, 0, 0);

$window->signal_connect ("delete-event" => sub { Gtk2->main_quit });
$window->set_title("Brum");
$window->set_wmclass("submit", "Submit");
$window->set_default_size(320, 400);
$window->add ($bbox);
$window->show_all;

#$window->window->set_cursor($cursor);
#$window->window->set_cursor(undef);

Gtk2->main;
