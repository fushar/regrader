#!/usr/bin/perl
use strict;
use warnings;
use Glib qw(TRUE FALSE);
use Gtk2 -init;

my ($where,$taskdir,$home)=('/mo/public/templater/','/mo/templates/',$ENV{'HOME'});
my @tasks = map { s#.*/##; $_ } (glob "$taskdir/*");
my $contestName = "MO-P";

sub create_template($$$$) {
    my ($path,$task,$name,$ext)=@_;
    return "Project cannot be created!" unless -d $path;
    return "Specified project already exist!" if -d "$path/$name";
    mkdir "$path/$name" or return "Can not create project directory!";
    my %files = (
	c => {
		"$where/template-c.cbp" => "$task.cbp",
		"$where/template.layout" => "$task.layout",
		"$taskdir/$task/$task.c" => "$task.c",
	},
	cpp => {
		"$where/template-cpp.cbp" => "$task.cbp",
		"$where/template.layout" => "$task.layout",
		"$taskdir/$task/$task.cpp" => "$task.cpp",
	},
	pas => {
		"$where/template-pas.lpi" => "${task}.lpi",
		"$taskdir/$task/$task.pas" => "$task.pas",
	}
    );
    my $files = $files{$ext} // die "Unknown ext $ext\n";
    while (my ($file, $target) = each %$files) {
	$target="$path/$name/$target";
	# FUJ! S tímhle by se něco mělo dělat.
	system("sed 's#fILENAMe#$name#;
                     s#fILENAMEEXt#$name.$ext#;
                     s#tASk#$task#;
                     s#tASKEXt#$task.$ext#;
		     s#pATHFILENAMe#$path/$name/$name#;
		     s#pATHFILENAMEEXt#$path/$name/$name.$ext#;
		    ' < '$file' > '$target'");
    }
    return "";
}

sub open_project($) {
    my ($p)=@_;
    my $cmd;
    my @files = glob "$home/$p/*.cbp";
    if (@files) {
	$cmd = 'codeblocks';
    } else {
        $cmd = 'lazarus-ide';
        @files = ((glob "$home/$p/*.lpi"), (glob "$home/$p/*.pas"));
    }
    print("@files\n");
    system("$cmd @files &");
}

sub existing_projects($) {
    my ($lopen_data,$lang)=@_;
    foreach ((glob "$home/*/*.cbp"), (glob "$home/*/*.lpi")) {
	my ($name, $task) = m#.*/(.*?)/(.*)(\.lpi|\.cbp)#;
	print "$name $task\n";
	$lang=-f "$home/$name/$task.c" ? 'C' : -f "$home/$name/$task.cpp" ? 'C++' : -f "$home/$name/$task.pas" ? 'Pascal' : '';
	$lopen_data->set($lopen_data->append, 0, "$1", 1, $lang);
    }
}

sub main_window() {
    my $retcode='';

    my $w=Gtk2::Window->new('toplevel');
    $w->signal_connect('destroy' => sub { $retcode='quit' unless length $retcode; Gtk2->main_quit; });
    $w->set_default_icon_from_file("$where/templater.xpm");
    $w->set_position('center');
    $w->set_default_size(600,-1);
    $w->set_title("$contestName project management");
    my $b=Gtk2::VBox->new(FALSE,0);
    $b->set_border_width(10);
    $w->add($b);

    my $fnew=Gtk2::Frame->new("Create new $contestName contest project");
    my $xnew=Gtk2::HBox->new(FALSE,0);
    $xnew->set_border_width(10);
    my $inew=Gtk2::Image->new_from_file("$where/templater.xpm");
    my $bnew=Gtk2::Button->new_with_label("Create new $contestName contest project");
    $bnew->signal_connect('clicked' => sub { $retcode='new'; $w->destroy; });
    $xnew->pack_start($inew,FALSE,TRUE,10);
    $xnew->pack_start($bnew,TRUE,TRUE,10);
    $fnew->add($xnew);
    $b->pack_start($fnew,FALSE,TRUE,10);

    my $fopen=Gtk2::Frame->new("Open existing $contestName contest project");
    my $yopen=Gtk2::VBox->new(FALSE,0);    $yopen->set_border_width(5);
    my $xopen=Gtk2::HBox->new(FALSE,0);    $xopen->set_border_width(5);
    my $lopen_data=Gtk2::ListStore->new(('Glib::String','Glib::String'));
    existing_projects($lopen_data);
    my $lopen=Gtk2::TreeView->new_with_model($lopen_data);
    my $lopen_c1=Gtk2::TreeViewColumn->new_with_attributes('Project name',Gtk2::CellRendererText->new, text=>0);    
    my $lopen_c2=Gtk2::TreeViewColumn->new_with_attributes('Language',Gtk2::CellRendererText->new, text=>1);
    $lopen_c1->set_expand(TRUE); $lopen->append_column($lopen_c1); $lopen->append_column($lopen_c2);
    my $iopen=Gtk2::Image->new_from_file("$where/templater.xpm");
    my $bopen=Gtk2::Button->new_with_label("Open existing $contestName contest project");
    $bopen->signal_connect('clicked' => sub { open_project $lopen_data->get(scalar($lopen->get_selection->get_selected),0),
                                              $w->destroy; });
    $bopen->set_sensitive(FALSE);
    $lopen->signal_connect('row-activated' => sub { $bopen->clicked });
    $lopen->get_selection->set_mode('single');
    $lopen->get_selection->signal_connect('changed' => sub { $bopen->set_sensitive(TRUE) } );
    $xopen->pack_start($iopen,FALSE,TRUE,10);
    $xopen->pack_start($bopen,TRUE,TRUE,10);
    $yopen->pack_start($lopen,TRUE,TRUE,5);
    $yopen->pack_start($xopen,FALSE,TRUE,5);
    $fopen->add($yopen);
    $b->pack_start($fopen,TRUE,TRUE,10);

    my $bquit=Gtk2::Button->new_from_stock('gtk-quit');
    $bquit->signal_connect('clicked' => sub { $w->destroy; });
    $b->pack_start($bquit,FALSE,TRUE,5);

    $w->show_all;
    Gtk2->main;
    $retcode;
}

sub message($$$) {
    my $m=Gtk2::MessageDialog->new($_[0], 'modal', $_[2], 'ok', $_[1]);
    $m->run;
    $m->destroy;
    1
}

sub new_window() {
    my $retcode='';

    my $w=Gtk2::Window->new('toplevel');
    $w->signal_connect('destroy' => sub { $retcode='quit' unless length $retcode; Gtk2->main_quit; });
    $w->set_default_icon_from_file("$where/templater.xpm");
    $w->set_position('center');
    $w->set_default_size(500,-1);
    $w->set_title("Create new $contestName project");

    my $b=Gtk2::VBox->new(FALSE,0);
    $b->set_border_width(10);
    $w->add($b);

    my $flang=Gtk2::Frame->new('Choose project language');
    my $xlang=Gtk2::HBox->new(FALSE,0);
    $xlang->set_border_width(10);
    my $r_c=Gtk2::RadioButton->new_with_label(undef, 'C');
    my $r_cpp=Gtk2::RadioButton->new_with_label($r_c->get_group(), 'C++');
    my $r_pas=Gtk2::RadioButton->new_with_label($r_c->get_group(), 'Pascal');
    $r_c->set_active(TRUE);
    $xlang->pack_start($r_c,TRUE,TRUE,5); 
    $xlang->pack_start($r_cpp,TRUE,TRUE,5);
    $xlang->pack_start($r_pas,TRUE,TRUE,5);
    $flang->add($xlang);
    $b->pack_start($flang,FALSE,TRUE,10);

    my $ftask=Gtk2::Frame->new('Choose task');
    my $xtask=Gtk2::HBox->new(FALSE,0);
    $xtask->set_border_width(10);
    my $group;
    my $task = $tasks[0];
    for my $t (@tasks) {
	    my $rb=Gtk2::RadioButton->new_with_label($group, $t);
	    $rb->signal_connect(clicked => sub {
		$task = $t;
	    });
	    unless ($group) {
		$rb->set_active(TRUE);
		$group=$rb->get_group();
	    }
	    $xtask->pack_start($rb,TRUE,TRUE,5);
    }
    $ftask->add($xtask);
    $b->pack_start($ftask,FALSE,TRUE,10);

    my $fprj=Gtk2::Frame->new('Enter project name');
    my $eprj=Gtk2::Entry->new;
    $fprj->add($eprj);
    $b->pack_start(Gtk2::HBox->new(),TRUE,TRUE,0);
    $b->pack_start($fprj,FALSE,TRUE,10);

    my $xcreate=Gtk2::HBox->new(FALSE,0);
    $xcreate->set_border_width(10);
    my $icreate=Gtk2::Image->new_from_file("$where/templater.xpm");
    my $bcreate=Gtk2::Button->new_from_stock('gtk-new');
    my $qcreate=Gtk2::Button->new_from_stock('gtk-cancel');
    $bcreate->signal_connect('clicked' => sub {
	my $p=$eprj->get_text; $p =~ s/^\s*//; $p =~ s/\s*$//;
	if ($p =~ /\W/) {
	    message($w, 'Sorry, but be nice with the project name!', 'error');
	    return;
	}
	if (not length $p) {
	    message($w, 'No project name specified!', 'error');
	    return;
	}
	my $ret=create_template $home, $task, $p, ($r_c->get_active ? 'c' : $r_cpp->get_active ? 'cpp' : 'pas');
	length $ret and message($w,$ret,'error') and return;
	open_project $p;
	$retcode='new';
	$w->destroy;
    });
    $qcreate->signal_connect('clicked' => sub { $w->destroy; });
    $xcreate->pack_start($icreate,FALSE,TRUE,10);
    $xcreate->pack_start($bcreate,TRUE,TRUE,10);
    $xcreate->pack_start($qcreate,TRUE,TRUE,10);
    $b->pack_start(Gtk2::HBox->new(),TRUE,TRUE,0);
    $b->pack_start($xcreate,FALSE,TRUE,10);
    
    $w->show_all;
    Gtk2->main;
    $retcode;
}

while (main_window eq 'new') {
    new_window eq 'quit' or exit 0;
}
