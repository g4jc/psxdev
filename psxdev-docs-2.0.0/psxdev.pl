#!/usr/bin/perl

# http://www.psxdev.de homepage generator +++

use File::stat;
use POSIX qw(strftime);

##################################
## PARAMETER SECTION

$doctype = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Strict//EN">';

$name		= "Balster";
$surname	= "Daniel";
$email		= 'dbalster@psxdev.de';
$year		= `date +%Y`;
$modified	= `date`;
$mailopts	= "";
$fullname	= "<a href=\"mailto:$email$mailopts\">$surname $name</a>";
$copyright	= "Copyright &copy;$year by $fullname";

$base       = "http://psxdev.de/";
$root		= "/tmp/psxdev";

$line		= "<hr size=2 noshade>";
$line75		= "<HR SIZE=1 WIDTH=95% NOSHADE>";

# contributors
$dbalster	= $fullname;
$sergio		= '<A HREF=sergio@x-plorer.co.uk>Sergio Moreira</A>';
$andrew		= '<A HREF=andrewk@cerc.utexas.edu>Andrew Kieschnick</A>';
$kazuki		= '<A HREF=bsd-ps@geocities.co.jp>Kazuki Sakamoto</A>';

#$workinprogress = "<CENTER><IMG SRC=images/work.png></CENTER>"


## colors

$bgcolor	= "#999999";
$fgcolor	= "#000000";
$alink		= "#003388";
$vlink		= "#770044";

$titlefg	= "#FFFFFF";
$titlebg	= "#000000";
$boxbg		= "#FFFFFF";
$boxbg1		= "#FFFFEE";
$boxbg2		= "#EEEEFF";
$grey		= "#EEEEEE";

# misc

$max_num_of_news = 10;

## data

@rpms = ("core",
	"libs", "libs-devel",
	"tools", "tools-for-audio", "tools-for-images",
	"media",
	"docs",
	"cdmaker", "debugger", "admin", "curvegen", "interactor",
	"binutils", "binutils-elf", "binutils-ecoff",
	"gcc",
	"egcs",
	"gimp");

@bins = (
"psx-run",
"psx-server",
"psx-resume",
"psx-halt",
"psx-show-registers",
"psx-bfd2x",
"psx-bin2x",
"psx-codedown",
"psx-codeup",
"psx-xaldown",
"psx-xaldump",
"psx-xalup",
"psx-upload",
"psx-download",
"psx-exe2mexe",
"psx-fbmode",
"psx-mode",
"psx-reset",
"psx-flash",
"mc-down",
"mc-up",
"mc-ls",
"mc-geticon",
"mc-mkfs",
"mc-rm",
"tim-down",
"tim-up",
"timsetpos",
"timtoppm",
"timmontage",
"oss-play",
"oss-query",
"oss-record",
"pcmtoadpcm",
"adpcmtopcm"
);

%bookmarks = (
# external bookmarks
"psxdev","http://psxdev.de/",
"actionreplay","http://www.datel.co.uk/",
"playstation","http://www.playstation.com/",
"freshmeat","http://freshmeat.net/",
"mandrake","http://www.linux-mandrake.com/",
"slashdot","http://slashdot.org/",
"caetla","http://www3.airnet.ne.jp/kcomm/",
"caetlaeng","http://www3.airnet.ne.jp/kcomm/cae/check.cgi?e",
"redhat","http://www.redhat.com/",
"datel","http://www.datel.co.uk/",
"linux","http://www.linux.com/",
"gimp","http://www.gimp.org/",
"google","http://www.google.com/",
"gtk", "http://www.gtk.org/",
"egcs", "http://egcs.cygnus.com/",
"gcc", "http://gcc.cygnus.com/",
"cdrdao", "http://www.ping.de/sites/daneb/cdrdao.html",
"mips","http://www.idt.com/",
"gnome","http://www.gnome.org/",
"mtsm","http://www.mountstuffmore.de/",
"glade","http://glade.pn.org/",
"sourceforge", "http://sourceforge.net",
"psxdev+sourceforge", "http://sourceforge.net/project?group_id=2053",

# internal bookmarks
"legal","legal.html",
"author","author.html",
"license","legal.html#license",
"copyright","legal.html#copyright",
"trademarks","legal.html#trademarks",
);
##################################

sub open_html {
	local ($name, $title) = @_;
	open(OUT,">$root/$name.html") || die "Couldn`t write to $name.html!\n";
	print OUT<<EOF
$doctype
<html lang=en><head>
<title>psxdev.de: $title</title>
<meta http-equiv="content-type" content="text/html;">
<link rel=stylesheet type="text/css" href="psxdev.css">
</head><body background="images/bg.png" color=$fgcolor bgcolor=$bgcolor>
<table width=100% cellpadding=0 cellspacing=0 border=0><tr><td align=left>$title</TD><TD ALIGN=RIGHT>[<A HREF=index.html>top</A>|<A HREF=overview.html>sitemap</A>|<A HREF=download.html>download</A>|<A HREF=documentation.html>docs</A>]</TD></TR></TABLE>
$line
EOF
}

#onload="install()"
#<script language="JavaScript"src="floating.js"></script>
#<span id="move" name="move"><img name="advert" src=images/werbung.gif></span>

##################################

sub close_html {
	print OUT<<EOF
$workinprogress
$line
<table width=100% cellpadding=0 cellspacing=0 border=0><tr><td align=left>
<address>Created with <a href=psxdev.pl>psxdev.pl</a></address>
</td><td align=right>
<address>$copyright</address>
</td></tr></table>
</body></html>
EOF
;	close(OUT);
}

##################################

sub make_headline {
	local ($headline) = @_;

#<CENTER><IMG SRC=Banner/gnome.gif></CENTER>

	print OUT<<EOF
<center><img src=images/psxtux2.png border=0 width=78 height=90><img src=images/logo.png width=496 height=90></center>
<h1>$headline</h1>
$workinprogress
EOF
}

sub start_left {
	print OUT<<EOF
<hr width=0 size=0><table width=100% valign=top><tr><td valign=top>
EOF
}

sub start_right {
	print OUT<<EOF
<hr width=0 size=0></td><td valign=top>
EOF
}

sub close_all {
	print OUT<<EOF
</td></tr></table>
EOF
}

sub open_table {
	print OUT "<hr width=0 size=0><table width=100% valign=top><tr valign=top><td valign=top>"
}

sub close_table {
	print OUT "</td></tr></table><hr width=0 size=0>"
}

sub open_column {
	local ($title) = @_;

print OUT<<EOF
<table width=100% cellspacing=0 cellpadding=1 border=0 bgcolor="#000000"><tr><td valign=center>
<table cellspacing=1 cellpadding=2 width=100% border=0>
<tr bgcolor=$titlebg><td width=100%>
<p class=title><img src=images/star.png>$title</p></td></tr><tr><td bgcolor=$boxbg>
EOF

}

sub close_column {
	print OUT "</td></tr></table></td></tr></table></td><td>"
}

sub close_column_v {
	print OUT "</td></tr></table></td></tr></table><br>"
}


##################################

sub make_box_for_rpm {
	local ($rpm) = @_;

@result = `rpm -q --queryformat="\"%{NAME}\"\n\"%{VERSION}-%{RELEASE}\"\n\"%{PROVIDENAME}\"\n\"%{REQUIRENAME}\"\n\"%{SUMMARY}\"\n\"%{GROUP}\"\n\"%{PACKAGER}\"\n\"%{DESCRIPTION}\"\n" $rpm`;
	
print OUT<<EOF
<table  width=100% cellspacing=0 cellpadding=1 border=0 bgcolor="#000000"><tr><td valign=center><table cellspacing=1 cellpadding=2 width=100% border=0>
<tr bgcolor=$titlebg><td width=100% colspan=2>
<p class=title><img src=images/star.png>@result[0]</p></td></tr>
<tr><td bgcolor=$boxbg colspan=2>
@result[7..$#result]
<p align=right>
<A HREF=download.html>download</A> or read <A HREF=$rpm.html>more details</A>
</P>
</TD></TR><TR BGCOLOR=$boxbg1><TD><B>VERSION</B></TD><TD WIDTH=100%>
@result[1]
</TD></TR><TR BGCOLOR=$boxbg2><TD><B>GROUP</B></TD><TD WIDTH=100%>
@result[5]
</TD></TR><TR BGCOLOR=$boxbg1><TD><B>REQUIRES</B></TD><TD WIDTH=100%>
@result[3]
</TD></TR><TR BGCOLOR=$boxbg2><TD><B>PROVIDES</B></TD><TD WIDTH=100%>
@result[2]
</TD></TR><TR BGCOLOR=$boxbg1><TD><B>SUMMARY</B></TD><TD WIDTH=100%>
@result[4]
</TD></TR><TR BGCOLOR=$boxbg2><TD><B>PACKAGER</B></TD><TD WIDTH=100%>
@result[6]
</td></tr></table></td></tr></table><hr width=0 size=0>
EOF
}

##################################

## rpm query stuff

sub query_rpm {
	local ($tag, $rpm) = @_;
	$x = `rpm -q --queryformat=\"%{$tag}\" $rpm`;
	$x =~ s/\n/, /g;
	return $x;
}

sub query_rpm_p {
	local ($tag, $rpm) = @_;
	$x = `rpm -qp --queryformat=\"%{$tag}\" $rpm`;
	$x =~ s/\n/, /g;
	return $x;
}

sub is_rpm_installed { return ; }

##################################

sub open_box {
	local ($span, $title) = @_;
	$dlbox_idx = 0;

chop($title);

print OUT<<EOF
<TABLE  WIDTH=100% CELLSPACING=0 CELLPADDING=1 BORDER=0 BGCOLOR="#000000"><TR><TD VALIGN=center><TABLE CELLSPACING=1 CELLPADDING=2 WIDTH=100% BORDER=0>
<TR BGCOLOR=$titlebg><TD WIDTH=100% COLSPAN=$span><p class=title><IMG SRC=images/star.png>$title</p></TD></TR>
EOF
}


sub add_box_header {

	print OUT "<TR BGCOLOR=$grey>";
	for ($i=0;$i<@_[0];$i++)
	{
		print OUT "<TD><I>@_[($i+1)]</I></TD>";
	}

#print OUT<<EOF
#<TD><I>@_[1]</I></TD>
#<TD><I>@_[2]</I></TD>
#<TD><I>@_[3]</I>
#EOF

}

sub psxname {
	return $_
}

sub open_box_no_title {
print OUT<<EOF
<table  width=100% cellspacing=0 cellpadding=1 border=0 bgcolor="#000000"><tr><td valign=center><table cellspacing=1 cellpadding=2 width=100% border=0>
<tr bgcolor=$boxbg><td width=100% valign=top>
EOF
}

sub close_box {
print OUT<<EOF
</table></td></tr></table><hr width=0 size=0>
EOF
}

sub add_download_box_entry {
	local ($name, $url, $size, $version, $desc) = @_;

	$col = ($dlbox_idx % 2)?$boxbg1:$boxbg2;
	$dlbox_idx++;

print OUT<<EOF
<tr bgcolor=$col><td><a href="download/$url">$name</a></td><td align=right>$size</td><td align=center>$version</td><td width=100%>$desc</td></tr>
EOF
}

sub add_colorized_box_entry {
	local ($name, $url) = @_;

	$col = ($dlbox_idx % 2)?$boxbg1:$boxbg2;
	$dlbox_idx++;

print OUT<<EOF
<tr bgcolor=$col><td><b><code>$name</code></b></td><td width=100%>$url</td></tr>
EOF
}

sub add_box_entry {
	local ($name) = @_;

print OUT "<tr bgcolor=$boxbg><td width=100%>"
}

sub add_box_entry2 {
	local ($name) = @_;

print OUT "<tr bgcolor=$boxbg><td>"
}

sub add_image {
	local ($name) = @_;

print OUT "<tr bgcolor=$boxbg><td><a href=images/$name.png><img src=images/s$name.png border=0></a></td><td width=100% valign=top>"
}

sub add_image_2 {
	local ($name,$image) = @_;

print OUT "<a href=$name.html><img src=images/$image.png border=0></a>"
}

sub add_image_jpg {
	local ($name) = @_;

print OUT "<tr bgcolor=$boxbg><td><a href=images/$name.jpg><img src=images/s$name.png border=0></a></td><td width=100% valign=top>"
}

sub close_table_row {
	print OUT "</td></tr>"
}

sub insert_link {
	local ($link) = @_;
print OUT "<a href=$bookmarks{$link}>$link</a>"
}

sub insert_link_2 {
	local ($name,$link) = @_;
print OUT "<li><a href=$link>$name</a><br>"
}

sub insert_image_link {
	local ($link) = @_;
	my $fn;
	
	$fn = "images/$link.png" if -e "images/$link.png";
	$fn = "images/$link.gif" if -e "images/$link.gif";
	$fn = "images/$link.jpg" if -e "images/$link.jpg";

print OUT "<a href=$bookmarks{$link}><img src=$fn border=0></a>"
}

################

$sh_keywords = "(new|delete|this|emit|connect|return|goto|if|else|case|default|switch|break|continue|while|do|for|sizeof)";
$sh_storage  = "(class|friend|virtual|inline|operator|template|overload|slots|signals|private|protected|const|extern|auto|register|static|unsigned|signed|volatile|char|double|float|int|long|short|void|bool|typedef|struct|union|enum|public|u_char|u_short|u_long|u_int|uchar|ulong|uint|ushort|quad|hyper|using|namespace)";
$sh_numeric  = "(0(x|X)[0-9a-fA-F]*)|([0-9]*)";
$sh_nullbool = "([A-Z][A-Z0-9_]+)";
$sh_stdctype = "([_a-z][0-9a-z_]+_[ts])";

sub highlight_source {
	local ($filename) = @_;

	open(HANDLE,$filename);
	
	print OUT "<ul><code><pre>\n";

	@all = <HANDLE>;

	$_ = join("",@all);

	s/</&lt;/g;
	s/>/&gt;/g;

#	s/\b${sh_numeric}\b/<font color=blue>$&<\/font>/g;
#	s/"[^"]+"/<span class=x1>$&<\/span>/g;
	s/\b${sh_keywords}\b/<b>$&<\/b>/g;
	s/\b${sh_storage}\b/<span class=y1>$&<\/span>/g;
	s/\b${sh_nullbool}\b/<span class=y3>$&<\/span>/g;
	s/\b${sh_stdctype}\b/<span class=x4>$&<\/span>/g;
	s/[{(\[\])}]/<b>$&<\/b>/g;
	s/"[^"]+"/<span class=x1>$&<\/span>/g;
	s{$\#[^\n]*}{<span class=x4>$&</span>}gs;
	s{\/\/[^\n]*}{<span class=y2>$&</span>}gs;
	s{/\*.*?\*/}{<span class=y2>$&</span>}gs;
	
	print OUT;
	print OUT "</pre></code></ul>";
	close(HANDLE);
}

################

$mips_reg = "(zero|at|gp|sp|fp|ra|sr|v[01]|a[0123]|t[0123456789]|s[01234567]|k[01])";
$mips_lab = "^([a-zA-Z_\$\.][a-zA-Z_0-9\$\.]+):";
$mips_insn = "^[ \t]+[a-zA-Z_][a-zA-Z_0-9]+";
$mips_dir = "^[ \t]+\.[a-zA-Z_][a-zA-Z_0-9]+";

sub highlight_asm {
	local ($filename) = @_;

	open(HANDLE,$filename);
	
	print OUT "<ul><code><pre>\n";

	@all = <HANDLE>;

	$_ = join("",@all);

	s/</&lt;/g;
	s/>/&gt;/g;

	s/${mips_lab}/<span class=y4>$&<\/span>/gsm;
	s/${mips_insn}/<b>$&<\/b>/gsm;
	s/${mips_reg}/<span class=y1>$&<\/span>/g;
	s/${mips_dir}/<span class=x5>$&<\/span>/gsm;
	s/"[^"]+"/<span class=x1>$&<\/span>/g;
	s{$\#[^\n]*}{<span class=x6>$&</span>}gs;
	s{/\*.*?\*/}{<span class=y2>$&</span>}gsm;
	s{\/\/[^\n]*}{<span class=y2$&</span>}gsm;
	
	print OUT;
	print OUT "</pre></code></ul>";
	close(HANDLE);
}

################

sub insert_file  {
	local ($fname) = @_;

	open(HANDLE2,$fname);

	while (<HANDLE2>)
	{
		if ($_ eq "<INCLUDE>\n")
		{
			$filename = <HANDLE2>;
			highlight_source ($filename);
		}
		elsif ($_ eq "<ASM>\n")
		{
			$filename = <HANDLE2>;
			highlight_asm ($filename);
		}
		else
		{
			print OUT;
		}
	}
	close (HANDLE2);
}





print "Creating filesystem\n";
system ("rm -rf $root");
mkdir ("$root",0755);
mkdir ("$root/images",0755);
mkdir ("$root/download",0755);
mkdir ("$root/contrib",0755);
mkdir ("$root/infos",0755);

print "Transferring data files\n";
system("ln -sf `pwd`/images/* $root/images");
system("ln -sf /home/dbalster/psxdev/RPMS/*/psxdev-* $root/download");
system("ln -sf /home/dbalster/psxdev/SRPMS/psxdev-* $root/download");
system("ln -sf /home/dbalster/psxdev/SOURCES/psxdev-* $root/download");
system("ln -sf /home/dbalster/psxdev/patches/* $root/download");

system("ln -sf `pwd`/contrib/* $root/contrib/");
system("ln -sf `pwd`/infos/* $root/infos/");
system("ln -sf `pwd`/psxdev.pl $root/");
system("ln -sf `pwd`/floating.js $root/");
system("ln -sf `pwd`/COPYING $root/COPYRIGHT");
system("ln -sf `pwd`/../ChangeLog $root/ChangeLog");

print "Creating pages\n";

###############################################################
###
### psxdev.css
###
###############################################################
print "psxdev.css\n";

open(FILE,">$root/psxdev.css") || die "Couldn`t write to psxdev.css!\n";
print FILE<<EOF
body {
	font-family: helvetica, arial, sans-serif;
	background-color:$bgcolor;
	color:$fgcolor;
	margin-top: 0pt;
}
a:link {
	text-decoration:none;
	color:$alink;
	font-weight:bold;
}
a:visited,a:active {
	text-decoration:none;
	color:$vlink;
	font-weight:bold;
}
.title {
	color:$titlefg;
	font-weight:bold;
}
#move {
	position: absolute;
	top:-1000px;
	left:0px;
}
.x1 { color:#ff0000; }
.x2 { color:#008800; }
.x3 { color:#880088; }
.x4 { color:#000088; }
.x5 { color:#888800; }
.x6 { color:#0000ff; }
.y1 { color:#880000; font-weight:bold; }
.y2 { color:#008800; font-weight:bold; }
.y3 { color:#880088; font-weight:bold; }
.y4 { color:#000088; font-weight:bold; }
EOF
;close(FILE);

###############################################################
###
### index.html
###
###############################################################
print "index.html\n";

open_html ("index","Welcome to PSXDEV!");
make_headline("Welcome to PSXDEV!");

open_table ();

open_box_no_title ();
print OUT<<EOF
<CENTER><FONT SIZE=+2>
<A HREF=whatispsxdev.html>
WHAT<BR>
IS<BR>
PSXDEV
</A></font><br>
<FONT SIZE=-2>click above for more informations</FONT>
</center>
EOF
;close_box();

open_column ("MY LINKS");
print OUT "<LI>";insert_link ("legal");print OUT "<BR>";
print OUT "<LI>";insert_link ("author");print OUT "<BR>";
print OUT "<LI>";insert_link ("license");print OUT "<BR>";
print OUT "<LI>";insert_link ("copyright");print OUT "<BR>";
print OUT "<LI>";insert_link ("trademarks");print OUT "<BR>";
close_column_v ();

open_column ("COOL LINKS");
print OUT "<LI>";insert_link ("actionreplay");print OUT "<BR>";
print OUT "<LI>";insert_link ("playstation");print OUT "<BR>";
print OUT "<LI>";insert_link ("freshmeat");print OUT "<BR>";
print OUT "<LI>";insert_link ("mandrake");print OUT "<BR>";
print OUT "<LI>";insert_link ("slashdot");print OUT "<BR>";
print OUT "<LI>";insert_link ("cdrdao");print OUT "<BR>";
print OUT "<LI>";insert_link ("redhat");print OUT "<BR>";
print OUT "<LI>";insert_link ("gnome");print OUT "<BR>";
print OUT "<LI>";insert_link ("caetla");print OUT "<BR>";
print OUT "<LI>";insert_link ("datel");print OUT "<BR>";
print OUT "<LI>";insert_link ("linux");print OUT "<BR>";
print OUT "<LI>";insert_link ("egcs");print OUT "<BR>";
print OUT "<LI>";insert_link ("gcc");print OUT "<BR>";
print OUT "<LI>";insert_link ("gimp");print OUT "<BR>";
print OUT "<LI>";insert_link ("mips");print OUT "<BR>";
print OUT "<LI>";insert_link ("gtk");print OUT "<BR>";
close_column_v ();

open_column ("RECOMMENDED");
print OUT "<CENTER>";
#insert_image_link ("psxdev");print OUT "<BR>";
insert_image_link ("gimp");print OUT "<BR>";
insert_image_link ("linux");print OUT "<BR>";
insert_image_link ("sourceforge");print OUT "<BR>";
insert_image_link ("gnome");print OUT "<BR>";
insert_image_link ("redhat");print OUT "<BR>";
print OUT "</CENTER>";
;close_column_v ();

open_column ("AWARDS");
insert_image_link ("mtsm");print OUT "<BR>";
;close_column_v ();

open_column ("QUICK LINKS");
insert_link_2 ("PSXDEV FAQ","FAQ.html");
insert_link_2 ("PSXDEV Core","psxdev-core.html");
insert_link_2 ("PSXDEV Docs","documentation.html");
insert_link_2 ("PSXDEV Libs","psxdev-libs.html");
insert_link_2 ("PSXDEV Apps","psxdev-apps.html");
insert_link_2 ("PSXDEV Tools","psxdev-tools.html");
insert_link_2 ("PSXDEV GIMP","psxdev-gimp.html");
;close_column ();

## TODO: show only the top ten news
##

open_column ("TOP NEWS");
my $num_news = 0;
foreach $_ (`ls -r ~/psxdev/data/news/[0-9]*`)
{
	# only the top five are presented here, all others on
	# a separate page.
	$num_news++;
	last unless ($num_news ne $max_num_of_news);
	open(FH,"$_");
	@file = <FH>;
	$name = substr(@file[1],0,index(@file[1],'<')-1);
	$mail = substr(@file[1],index(@file[1],'<')+1);
	chop($mail);chop($mail);
	chop(@file[0]);
	chop(@file[2]);
	chop(@file[3]);
	
	$adate = strftime("%A, %b %e @ %H:%M %Z %Y",gmtime(@file[2]));
	
print OUT<<EOF
<table width=100% cellpadding=2 cellspacing=0 border=0 bgcolor=$grey><tr VALIGN=center><td align=left>
<B>@file[0]</B></td><td align=right>
<FONT SIZE=-2>from <A HREF="mailto:$mail">$name</A><br> $adate</FONT></td></tr></table>
@file[4..$#file]
EOF
;
	if (@file[3] ne "") {
	print OUT "<P ALIGN=RIGHT><A HREF=@file[3]>more...</A></P>"
	}
	close(FH);
}
close_column_v ();

print OUT "<P ALIGN=RIGHT><A HREF=news.html>click here for all news...</A></P>";

## this is just a test

close_table ();
close_html ();

###############################################################
###
### whatispsxdev.html
###
###############################################################
print "whatispsxdev.html\n";

open_html ("whatispsxdev","What is PSXDEV?");
make_headline("What is PSXDEV ?");

open_box_no_title ();
print OUT<<EOF
<p>
Back in 1997 I (<i>$fullname</i>) found various informations
about PSX internals in the internet and started to write my very
own development environment.
</p>
EOF
;insert_file("text/what_is_psxdev.html");
print OUT<<EOF
<p>
I have to thank these people for various ideas, contributions and support:
<ul>
<li>$sergio<li>$andrew<li>$kazuki
</ul>
</p>
<br>
EOF
;close_box ();
close_html ();

###############################################################
###
### overview.html
###
###############################################################
print "overview.html\n";

open_html ("overview","Site overview");
make_headline("Site overview");

open_box_no_title ();
print OUT<<EOF
This is a small graphical representation of <A HREF=http://psxdev.de>psxdev.de</A> which allows you
to quickly navigate through the site.
EOF
;close_box ();

print OUT "<center>";
print OUT `cat text/overview.map`;
print OUT "</center>";

open_box_no_title ();
print OUT<<EOF
Some more information here
EOF
;close_box ();


close_html ();

###############################################################
###
### news.html
###
###############################################################
print "news.html\n";

open_html ("news","News archive");
make_headline("News archive");
open_table ();
open_column ("ALL NEWS");
my $num_news = 0;
foreach $_ (`ls -r ~/psxdev/data/news/[0-9]*`)
{
	open(FH,"$_");
	@file = <FH>;
	$name = substr(@file[1],0,index(@file[1],'<')-1);
	$mail = substr(@file[1],index(@file[1],'<')+1);
	chop($mail);chop($mail);
	chop(@file[0]);
	chop(@file[2]);
	chop(@file[3]);

	$adate = strftime("%A, %b %e @ %H:%M %Z %Y",gmtime(@file[2]));

print OUT<<EOF
<table width=100% cellpadding=2 cellspacing=0 border=0 bgcolor=$grey><tr VALIGN=top><td align=left>
<B>@file[0]</B></td><td align=right valign=center>
<FONT SIZE=-2>from <A HREF="mailto:$mail">$name</A><br> $adate</FONT></td></tr></table>
@file[4..$#file]
EOF
;
	if (@file[3] ne "") {
	print OUT "<P ALIGN=RIGHT><A HREF=@file[3]>more...</A></P>"
	}
	close(FH);
}
close_column_v ();
close_table ();
close_html ();

###############################################################
###
### installation.html
###
###############################################################
print "installation.html\n";

open_html ("installation","Installation guide");
make_headline("Installation guide");

open_box_no_title ();
insert_file("text/you_need_rpm.html");
close_box ();

open_column ("Installing PSXDEV from binary RPMS");
insert_file("text/install_bin_rpms.html");
close_column_v ();

open_column ("Installing PSXDEV from source RPMS");
insert_file("text/install_src_rpms.html");
close_column_v ();

open_column ("Installing PSXDEV from source TARBALLS");
insert_file("text/install_tarballs.html");
close_column_v ();

close_all();
close_html ();

#goto skip;

###############################################################
###
### RPM pakets.html
###
###############################################################
print "pakets.html\n";

open_html ("packets","Description of RPM packets");
make_headline("Automatically generated RPM query report");

foreach $_ (@rpms)
{
	my $rpm = "psxdev-$_";

	# draw a box for each RPM which is installed
	if (`rpm -q $rpm`)
	{
		make_box_for_rpm ($rpm);
	}
}

close_all();
close_html ();

###############################################################
###
### download.html
###
###############################################################
print "download.html\n";

open_html ("download","Download area");
make_headline("Download area");

open_box_no_title ();
insert_file("text/download.html");
;close_box ();

## binary RPMS

@files = `( cd $root/download && ls -1 psxdev*.i*86.rpm psxdev*.noarch.rpm)`;
open_box (4,"PSXDEV Binary RPMS");
add_box_header (4,"name","size","version","description");
foreach $_ (@files)
{
	chop();
	$file = $_;
	($psxdevname) = ( $file =~ /[a-z-]+/g );
	chop($psxdevname);
	($dummy,$name) = split("psxdev-",$psxdevname);
	($version) = ( $file =~ /(([0-9]+.[0-9]+.[0-9]+)(-[0-9]+(mdk)?)?)/g );
	$size = stat("$root/download/$file")->size;
	add_download_box_entry ($name,$file,"$size","$version",query_rpm_p("SUMMARY","$root/download/$file"));
}
close_box ();

## source RPMS

@files = `( cd $root/download && ls -1 psxdev*.src.rpm )`;
open_box (4,"PSXDEV Source RPMS");
add_box_header (4,"name","size","version","description");
foreach $_ (@files)
{
	chop();
	$file = $_;
	($psxdevname) = ( $file =~ /[a-z-]+/g );
	chop($psxdevname);
	($dummy,$name) = split("psxdev-",$psxdevname);
	($version) = ( $file =~ /(([0-9]+.[0-9]+.[0-9]+)(-[0-9]+(mdk)?)?)/g );
	$size = stat("$root/download/$file")->size;
	add_download_box_entry ($name,$file,"$size","$version",query_rpm_p("SUMMARY","$root/download/$file"));
}
close_box ();

## tar balls

@files = `( cd $root/download && ls -1 psxdev*.tar.?z* )`;
open_box (4,"PSXDEV Tarballs");
add_box_header (4,"name","size","version","description");
foreach $_ (@files)
{
	chop();
	$file = $_;
	($psxdevname) = ( $file =~ /[a-z-]+/g );
	chop($psxdevname);
	($dummy,$name) = split("psxdev-",$psxdevname);
	($version) = ( $file =~ /(([0-9]+.[0-9]+.[0-9]+)(-[0-9]+(mdk)?)?)/g );
	$size = stat("$root/download/$file")->size;
	add_download_box_entry ($name,$file,"$size","$version","unconfigured source code only");
}
close_box ();

## patches

@files = `( cd $root/download && ls -1 psxdev*.patch* )`;
open_box (4,"PSXDEV Patches");
add_box_header (4,"name","size","version","description");
foreach $_ (@files)
{
	chop();
	$file = $_;
	($psxdevname) = ( $file =~ /[a-z-]+/g );
	chop($psxdevname);
	($dummy,$name) = split("psxdev-",$psxdevname);
	($version) = ( $file =~ /(([0-9]+.[0-9]+.[0-9]+)(-[0-9]+(mdk)?)?)/g );
	$size = stat("$root/download/$file")->size;
	add_download_box_entry ($name,$file,"$size","$version","unconfigured source code only");
}
close_box ();

## MD5 Sums

@files = `( cd $root/download && md5sum * )`;
open_box (2,"PSXDEV MD5 Checksums");
add_box_header (2,"MD5 checksum","filename");
foreach $_ (@files)
{
	chop;
	split(' ');
	add_colorized_box_entry ($_[0],$_[1]);
}
close_box ();

close_all();
close_html ();

skip:

###############################################################
###
### psxdev-core.html
###
###############################################################
print "psxdev-core.html\n";

open_html ("psxdev-core","psxdev-core");
make_headline("psxdev core - device driver and filesystem");

open_box_no_title ();
print OUT<<EOF
Currently nothing important to type...
EOF
;close_box ();

open_column ("PSXDEV core components");
insert_file("text/core_components.html");
close_column_v ();

open_column ("The /proc/pccl process information pseudo-filesystem");
insert_file("text/proc_pccl.html");
close_column_v ();

open_column ("The /dev/pccl kernel device driver interface");
insert_file("text/dev_pccl.html");
close_column_v ();

close_html ();

###############################################################
###
### pccl_api.html
###
###############################################################
print "pccl_api.html\n";

open_html ("pccl_api","PCCL API reference");
make_headline("PCCL API reference guide and documentation");

open_box_no_title ();
print OUT<<EOF
This is the offical PCCL API reference guide. It documents all
commands and their parameter packets. Using the /dev/pccl device driver
prevents you from using a direct I/O port based programming interface
and simplifies a lot.
EOF
;close_box ();

open_column ("/dev/pccl I/O command reference");
my $num_news = 0;

open(FH,"< text/pccl.refs");

while (<FH>)
{
	$name = $_;

	$dir = <FH>;
	$type = <FH>;
	$class = <FH>;
	$desc = <FH>;

	chop($name);
	chop($dir);
	chop($type);
	chop($class);
	chop($desc);
	
print OUT<<EOF
<table width=100% cellpadding=2 cellspacing=0 border=0 bgcolor=$grey>
<tr VALIGN=top><td align=left>
<B>$name</B></td><td align=right>
<FONT SIZE=-1>$class</FONT></td></tr>
<tr bgcolor=$boxbg1>
<td align=left><I>$desc</I></td>
<td align=right>
<B>$type : $dir</B>
</td>
</tr>
</table>
EOF
;

	while (<FH>)
	{
		last unless ($_ ne "EOF\n");
		print OUT "$_";
	}

	
}
close(FH);
close_column_v ();


open_column ("/dev/pccl data types and structures");
insert_file("text/pccl_api1.html");
close_column_v ();

close_html ();

###############################################################
###
### psxdev-libs.html
###
###############################################################
print "psxdev-libs.html\n";

open_html ("psxdev-libs","psxdev-libs");
make_headline("psxdev libs - shared libraries");

open_box_no_title ();
print OUT<<EOF
Here is the programming documentation for the PSXDEV shared libraries.
This documentation is not yet completed.
EOF
;close_box ();

open_column ("psxdev server library");
insert_file("text/libpsxdev.html");
close_column_v ();

open_column ("TIM image format library");
insert_file("text/libtim.html");
close_column_v ();

open_column ("BS image format library");
insert_file("text/libbs.html");
close_column_v ();

close_html ();

###############################################################
###
### documentation.html
###
###############################################################
print "documentation.html\n";

open_html ("documentation","Documentation");
make_headline("PSXDEV Documentation");

open_box_no_title();
print OUT<<EOF
Here are grouped links to all available documentation files. If you
have a free contribution to make, just send me the document and I will
add it here.
EOF
;close_box();

open_table ();

open_column ("APPLICATIONS");
insert_link_2 ("Debugger","psxdebug.html");
insert_link_2 ("CD Maker","psxcdmaker.html");
insert_link_2 ("Interactor","psxinteractor.html");
insert_link_2 ("CurveGen","psxcurvegen.html");
insert_link_2 ("Webtool","psxwebtool.html");
insert_link_2 ("PSX mkisofs","psxmkisofs.html");
close_column_v ();

#open_column ("PUBLIC INFOS");
#insert_link_2 ("PSX System (ENG)","infos/system.txt");
#print OUT "<P align=right><font size=-2>from http://psx.rules.org</font></p>";
#insert_link_2 ("PSX GPU (ENG)","infos/gpu.txt");
#print OUT "<P align=right><font size=-2>from http://psx.rules.org</font></p>";
#insert_link_2 ("PSX GTE (ENG)","infos/gte.txt");
#print OUT "<P align=right><font size=-2>from http://psx.rules.org</font></p>";
#insert_link_2 ("PSX SPU (ENG)","infos/spu.txt");
#print OUT "<P align=right><font size=-2>from http://psx.rules.org</font></p>";
#close_column ();


open_column ("TUTORIALS");
insert_link_2 ("Programming","tutorial_linux.html");
insert_link_2 ("PCCL API","pccl_api.html");
insert_link_2 ("PSXDEV FAQ","FAQ.html");
insert_link_2 ("Miscellaneous","miscellaneous.html");
close_column ();

open_column ("CONTRIBUTIONS");
insert_link_2 ("CAETLA Protocol (ENG)","contrib/capr_eng.txt");
print OUT "<P align=right><font size=-2>contributed by $kazuki</font></p>";
#insert_link_2 ("MDEC internals (JAP)","contrib/mdec.txt");
#print OUT "<P align=right><font size=-2>contributed by ????</font></p>";
close_column ();

open_column ("PACKETS");
insert_link_2 ("Description","packets.html");
insert_link_2 ("Download","download.html");
insert_link_2 ("CORE","psxdev-core.html");
insert_link_2 ("LIBS","psxdev-libs.html");
insert_link_2 ("TOOLS","psxdev-tools.html");
insert_link_2 ("APPS","psxdev-apps.html");
insert_link_2 ("GIMP","psxdev-gimp.html");
close_column ();

close_table ();
close_html ();


###############################################################
###
### tutorial_linux.html
###
###############################################################
print "tutorial_linux.html\n";

open_html ("tutorial_linux","tutorial for programmers");
make_headline("Tutorial for linux/psxdev programmers");

open_box_no_title ();
print OUT<<EOF
Here is a collection of tutorials and example code. It shows you
how to use the PCCL device driver and related interfaces, performing
data conversion and all concerns of communication with the remote machine
(the PlayStation). You should be very familar with the C/C++ programming
language. It also has some GNOME tutorials, which will help you embedding
the PSXDEV Gtk+ widgets in your own applications (like the R3kDisassembler).
EOF
;close_box ();

open_box (2,"Available tutorials");
add_box_header (2,"no.", "title");

my $index = 1;
foreach $_ (`ls -r tutorial/*`)
{
	open(FH,"$_");
	if (<FH> eq "<TUTORIAL TYPE=PSXDEV>\n")
	{
		$name = <FH>;
		chop ($name);
		$author = <FH>;
		chop ($author);
		$short = <FH>;
		chop ($short);
				
		$link = "<A HREF=$short.html>$name</A>";
		add_colorized_box_entry ($index,$link);
		$index++;
	}
	close (FH);
}
close_box ();
close_html ();

my $index = 1;
foreach $_ (`ls -r tutorial/*`)
{
	open(FH,"$_");
	if (<FH> eq "<TUTORIAL TYPE=PSXDEV>\n")
	{
		$name = <FH>;
		chop ($name);
		$author = <FH>;
		chop ($author);
		$short = <FH>;
		chop ($short);

		open_html ($short,$name);
		make_headline($name);

		while (<FH>)
		{
			if ($_ eq "<TBOX>\n")
			{
				$title = <FH>;
				
				open_column ($title);
			}
			elsif ($_ eq "<BOX>\n")
			{
				open_box_no_title ();
			}
			elsif ($_ eq "</BOX>\n")
			{
				close_box ();
			}
			elsif ($_ eq "</TBOX>\n")
			{
				close_column_v ();
			}
			elsif ($_ eq "<INCLUDE>\n")
			{
				$filename = <FH>;
				highlight_source ($filename);
			}
			elsif ($_ eq "<ASM>\n")
			{
				$filename = <FH>;
				highlight_asm ($filename);
			}
			else
			{
				print OUT "$_";
			}
		}

		close_html();
	}
	close(FH);
}




###############################################################
###
### misc documents
###
###############################################################
print "misc documents:\n";

open_html ("miscellaneous","misc documents");
make_headline("Miscellaneous documents");

open_box_no_title ();
print OUT<<EOF
Miscellaneous documents
EOF
;close_box ();

open_box (2,"Available documents");
add_box_header (2,"no.", "title");

my $index = 1;
foreach $_ (`ls -r misc/*`)
{
	open(FH,"$_");
	if (<FH> eq "<MISC TYPE=PSXDEV>\n")
	{
		$name = <FH>;
		chop ($name);
		$author = <FH>;
		chop ($author);
		$short = <FH>;
		chop ($short);
				
		$link = "<A HREF=$short.html>$name</A>";
		add_colorized_box_entry ($index,$link);
		$index++;
	}
	close (FH);
}
close_box ();
close_html ();

my $index = 1;
foreach $_ (`ls -r misc/*`)
{
	open(FH,"$_");
	if (<FH> eq "<MISC TYPE=PSXDEV>\n")
	{
		$name = <FH>;
		chop ($name);
		$author = <FH>;
		chop ($author);
		$short = <FH>;
		chop ($short);

		open_html ($short,$name);
		make_headline($name);

		while (<FH>)
		{
			if ($_ eq "<TBOX>\n")
			{
				$title = <FH>;
				
				open_column ($title);
			}
			elsif ($_ eq "<BOX>\n")
			{
				open_box_no_title ();
			}
			elsif ($_ eq "</BOX>\n")
			{
				close_box ();
			}
			elsif ($_ eq "</TBOX>\n")
			{
				close_column_v ();
			}
			elsif ($_ eq "<INCLUDE>\n")
			{
				$filename = <FH>;
				highlight_source ($filename);
			}
			elsif ($_ eq "<ASM>\n")
			{
				$filename = <FH>;
				highlight_asm ($filename);
			}
			else
			{
				print OUT "$_";
			}
		}

		close_html();
	}
	close(FH);
}




###############################################################
###
### psxdev-tools.html
###
###############################################################
print "psxdev-tools.html\n";

open_html ("psxdev-tools","psxdev-tools");
make_headline("psxdev tools - command line tools");

open_box_no_title ();
print OUT<<EOF
All these tools are command line based. You can use them perfectly in Makefiles
batch scripts. All utilties support GNU style long and short options, you
can always expect the options <i>--help</i>, <i>--version</i>, <i>--verbose</i> and
<i>--quiet</i> to be present.
EOF
;close_box ();

open_box (2,"Index of manual pages");
add_box_header (2,"command","description");
foreach $_ (`ls -r ~/psxdev/data/manpages/*`)
{
	open(FH,"$_");
	if (<FH> eq "<MANPAGE>\n")
	{
		$cmd = <FH>;
		$desc = <FH>;
		chop ($cmd);
		chop ($desc);
		$link = "<A HREF=$cmd.html>$cmd</A>";
		add_colorized_box_entry ($link,$desc);
	}
	close (FH);
}
close_box ();

close_html ();

# automatically generated


foreach $_ (`ls -r ~/psxdev/data/manpages/*`)
{
	open(FH,"$_");
	if (<FH> eq "<MANPAGE>\n")
	{
		@file = <FH>;
		$name = substr(@file[2],0,index(@file[2],'<')-1);
		$mail = substr(@file[2],index(@file[2],'<')+1);
		chop($mail);chop($mail);

		chop(@file[0]);
		chop(@file[1]);

	open_html (@file[0],@file[1]);
	make_headline(@file[1]);
	open_box_no_title ();

	print OUT<<EOF
<table width=100% cellpadding=2 cellspacing=0 border=0 bgcolor=$grey><tr VALIGN=top><td align=left>
<B>@file[0]</B> - @file[1]</td><td align=right>
<FONT SIZE=-2><A HREF="mailto:$mail">$name</A>/@file[3]</FONT></td></tr></table>
EOF
;
		print OUT ("<ul><li><b>SYNOPSIS OF @file[0]</b><br>\n");
		
		@syn = `@file[0] -h 2>&1`;
		# search "Usage"
		$i=0;
		for (@syn)
		{
			$i++;
			last if (/^Usage/);
		}
		
		print OUT "<CODE><PRE>\n";
		
		for (;$i < @syn; $i++)
		{
			$elem = @syn[$i];
			chop($elem);
			print OUT "$elem\n";
		}
		
		print OUT ("</PRE></CODE></ul>\n");

		open (FH2,"$_");
		while (<FH2>)
		{
			if ($_ eq "<DESCRIPTION>\n")
			{
				print OUT ("<ul><li><b>DESCRIPTION OF @file[0]</b><br>\n");
				while (<FH2>)
				{
					last unless ($_ ne "</DESCRIPTION>\n");
					s/XXX/<B>@file[0]<\/B>/g;
					print OUT "$_";
				}
				print OUT ("</ul>\n");
			}
			elsif ($_ eq "<EXAMPLES>\n")
			{
				print OUT ("<ul><li><b>EXAMPLES FOR @file[0]</b><br>\n");
				while (<FH2>)
				{
					last unless ($_ ne "</EXAMPLES>\n");
					print OUT "$_";
				}
				print OUT ("</ul>\n");
			}
			elsif ($_ eq "<BUGS>\n")
			{
				print OUT ("<ul><li><b>BUGS OF @file[0]</b><br>\n");
				while (<FH2>)
				{
					last unless ($_ ne "</BUGS>\n");
					s/XXX/<B>@file[0]<\/B>/g;
					print OUT "$_";
				}
				print OUT ("</ul>\n");
			}
			elsif ($_ eq "<NOTES>\n")
			{
				print OUT ("<ul><li><b>NOTES ON @file[0]</b><br>\n");
				while (<FH2>)
				{
					last unless ($_ ne "</NOTES>\n");
					s/XXX/<B>@file[0]<\/B>/g;
					print OUT "$_";
				}
				print OUT ("</ul>\n");
			}
		}
		close (FH2);
		
		chop (@file[4]);
				
		if (@file[4] ne "")
		{
			print OUT ("<ul><li><b>SEE ALSO</b><br>\n");
			foreach $_ (split(", ","@file[4]"))
			{
				print OUT "<A HREF=$_.html>$_</A>&nbsp;&nbsp;&nbsp;";
			}
		}
		
		print OUT "<p>&nbsp;</p>";
		close_box();
		print OUT "<P ALIGN=CENTER><A HREF=psxdev-tools.html>back to index</A></P>";
		close_html();
	}
	close(FH);
}

###############################################################
###
### psxdev-egcs.html
###
###############################################################
print "psxdev-gcc.html\n";

open_html ("psxdev-gcc","psxdev-gcc");
make_headline("psxdev gcc - GNU C/C++ compiler");

open_box_no_title ();
print OUT<<EOF
TODO: write more documentation
EOF
;close_box ();

# automatically generated

close_html ();

###############################################################
###
### psxdev-binutils.html
###
###############################################################
print "psxdev-binutils.html\n";

open_html ("psxdev-binutils","psxdev-binutils");
make_headline("psxdev binutils - GNU binary utilities");

open_box_no_title ();
print OUT<<EOF
TODO: write more documentation
EOF
;close_box ();

# automatically generated

close_html ();

###############################################################
###
### psxdev-gimp.html
###
###############################################################
print "psxdev-gimp.html\n";

open_html ("psxdev-gimp","psxdev-gimp");
make_headline("psxdev gimp - GIMP plug-ins");

open_column ("TIM plug-in");
print OUT<<EOF
This plug-in extends The Gimp to read and write
images in TIM format. The 24-bit mode is currently not
supported. This plug-in installs also some other new procedures to
the GIMP, please ignore them for now or check the source for more
informations.
EOF
;close_column_v ();

open_column ("BS plug-in");
print OUT<<EOF
This plug-in extends The Gimp to read and write
images in BS format. Please note:
Width, Height and Compression are controlled by the Toolbox/PSX/BS Config
dialog. The BS image format itself does not have informations about its
width or height.
EOF
;close_column_v ();

# automatically generated

close_html ();

###############################################################
###
### psxdev-apps.html
###
###############################################################
print "psxdev-apps.html\n";

open_html ("psxdev-apps","psxdev-apps");
make_headline("psxdev apps - some bigger applications");

#open_box_no_title ();
#print OUT<<EOF
#....
#EOF
#;close_box ();

open_table ();

open_column ("DEBUGGER");
add_image_2 ("psxdebug","sdebug4");
close_column_v ();

open_column ("CDMAKER");
add_image_2 ("psxcdmaker","scdmaker1");
close_column_v ();

open_column ("INTERACTOR");
add_image_2 ("psxinteractor","sinteractor");
close_column_v ();

open_column ("CURVEGEN");
add_image_2 ("psxcurvegen","scurvegen");
close_column ();

open_column ("PSXDEV APPLICATIONS");
print OUT<<EOF
PSXDEV is now being populated with lots of applications. All applications
with a graphical user interface are using
<A HREF=$bookmarks{gnome}>GNOME</A> and/or <A HREF=$bookmarks{gtk}>GTK+</A>
as the favorite GUI system. This has several reasons:
<ul>
<li>Gtk+ is very easy to use and very powerful
<li>The existance of the excellent XML based GUI builder <A HREF=$bookmarks{glade}>GLADE</A>
<li>The GIMP and its plug-ins are using it
<li>It is totally customizable
<li>There is an OpenGL widget
<li>It is free of charge and open sourced
</ul>
There are of course other good projects but not for PSXDEV.
EOF
;close_column ();

close_table ();

# automatically generated

close_html ();

###############################################################
###
### psxdebug.html
###
###############################################################
print "psxdebug.html\n";

open_html ("psxdebug","PSXDEV Debugger");
make_headline("PSXDEV Debugger - realtime remote target mips debugger");

open_box (2,"Screen shots");
add_box_header (2,"image","description");

add_image ("debug4");
print OUT<<EOF
An impressing screen shot, but be warned: every widget you see works as
expected. GNOME PSXDEV Debugger is already included in an 1.0 version
and can be found in the psxdev-apps package. Only these features are currently
not yet implemented: editing register contents (write back), hardware
breakpoints at all (only software) and breakpoint disabling (just delete
them and insert them again). COP2 instructions are painted red, I am not
sure if I have covered the full instruction set. The whole debugger was
written from scratch in about two days (during xmas 1999)! The R3K disassembly
is a 100% pure Gtk+ widget and can easily be used with MS Windows and has a
very powerful API.
EOF
;add_image ("debug2");
print OUT<<EOF
This is the upload tab of PSXDEV. If you provide a debugging version of
your executable, you are able to browse the symbolic informations and
can open an editor with the C source code by clicking on the assembler
instruction!
EOF
;add_image ("debug3");
print OUT<<EOF
You can set bookmarks for addresses or simply import the labels of your
application...
EOF
;add_image ("debug1");
print OUT<<EOF
Everything is configurable.
EOF
;add_image ("debug5");
print OUT<<EOF
The R3kDisasm widget has some unique features like hyperlink navigation,
including a graphical branch indicator if the mouse enters a branch.
EOF

;close_box ();

# automatically generated

close_html ();

###############################################################
###
### psxcdmaker.html
###
###############################################################
print "psxcdmaker.html\n";

open_html ("psxcdmaker","PSXDEV CD Maker");
make_headline("PSXDEV CD Maker -  CD-ROM mastering and writing utility");

open_box (2,"Screen shots");
add_box_header (2,"image","description");

add_image ("cdmaker1");
print OUT<<EOF
PSXDEV CD Maker has an integrated html based online help including a
wizard like tutorial. I am doing my best to write the best documentation
possible. Maybe I will integrate this widget in all my applications.
EOF
;add_image ("cdmaker2");
print OUT<<EOF
This is the path tab. Input and output paths are selected here, and you
can do disk space checks (used space for input, free space for output).
EOF
;add_image ("cdmaker3");
print OUT<<EOF
These are the settings for the base ISO 9660 filesystem.
EOF
;add_image ("cdmaker4");
print OUT<<EOF
Here are some PSXDEV special filesystem attributes. The XA inclusion list
lets you specify patterns, and all files in the filesystem which will match
against such a pattern will be written in XA mode (2336 bytes/sector).
So you do not need to single click each XA file each time you are creating
a new layout, a simple pattern like <i>*.xa</i> is sufficient.
EOF
;add_image ("cdmaker5");
print OUT<<EOF
These are the settings of the CD writer application which is used internally.
EOF
;add_image ("cdmaker6");
print OUT<<EOF
During all sub processes, a full featured shell terminal lets you interact
(if necessary; lets say CTRL-C or "press return") with the processes.
<i>psx-mkisofs and cdrdao are started as sub processes</i>
EOF
;add_image ("cdmaker7");
print OUT<<EOF
You can really configure everything. Even the GUI will be configurable with an
XML file!
EOF

;close_box ();

# automatically generated

close_html ();

###############################################################
###
### psxmkisofs.html
###
###############################################################
print "psxmkisofs.html\n";

open_html ("psxmkisofs","PSXDEV mkisofs");
make_headline("mkisofs - special version for PSXDEV");

open_box_no_title ();
print OUT<<EOF
To be written! This is an enhanced version of mkisofs, which has
support for XA files and license embedding.
EOF
;
close_box ();

close_html ();

###############################################################
###
### psxinteractor.html
###
###############################################################
print "psxinteractor.html\n";

open_html ("psxinteractor","PSXDEV Interactor");
make_headline("PSXDEV Interactor - artist debugger");

open_box_no_title ();
print OUT<<EOF
To be written. This is a realtime datatype interactor. Mainly
used to change color vectors in realtime.
EOF
;close_box ();

open_box (2,"Screen shots");
add_box_header (2,"image","description");

add_image ("interactor");
print OUT<<EOF
The interactor has to be started with a recent debugging version of your
current application. It retrieves than the symbolic information from it
and scans for all <b>global static</b> variables, which are the only ones
who have an absolute address. 
EOF
;add_image ("interactor2");
print OUT<<EOF
This is an example for a currently edited data type, a color vector.
EOF
;
close_box ();
close_html ();

###############################################################
###
### legal.html
###
###############################################################
print "legal.html\n";

open_html ("legal","PSXDEV legal, copyright, license, trademarks, ...");
make_headline("legal, copyright, license, trademarks, ...");

print OUT "<A NAME=#legal>";
open_column ("LEGAL");
print OUT<<EOF
<CODE><B>
YOU ARE NOT ALLOWED TO USE PSXDEV TO PRODUCE ANY KIND OF COMMERCIAL
CONTENT LIKE VIDEO GAMES, UNLESS YOU HAVE OBTAINED A LEGAL
DEVELOPMENT LICENSE AND/OR BECOME A REGISTERED DEVELOPER FOR THE
SONY COMPUTER ENTERTAINMENT PLAYSTATION PLATFORM.
</B></CODE>
EOF
;close_column_v ();
print OUT "</A><A NAME=#disclaimer>";
open_column ("DISCLAIMER");
print OUT<<EOF
<CODE><B>
THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
MERCHANTIBILITY AND FITNESS FOR ANY PARTICULAR PURPOSE.
THE AUTHORS HAVE NO OBLIGATIONS TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
</B></CODE>
EOF
;close_column_v ();
print OUT "</A><A NAME=#license>";
open_column ("LICENSE");
print OUT<<EOF
<CODE><B>
UNLESS OTHERWISE STATED, THIS SOFTWARE IS PROVIDED UNDER
THE TERMS OF <A HREF=COPYRIGHT>THE GNU GENERAL PUBLIC LICENSE VERSION 2</A>
</B></CODE>
EOF
;close_column_v ();
print OUT "</A><A NAME=#copyright>";
open_column ("COPYRIGHT");
print OUT<<EOF
<CODE><B>
Copyright 1996, 1997, 1998, 1999, 2000 by
<UL>
<LI> $dbalster
<LI> $sergio
<LI> $andrew
<LI> $kazuki
</UL>
</B></CODE>
EOF
;close_column_v ();
print OUT "</A><A NAME=#trademarks>";
open_column ("TRADEMARKS");
print OUT<<EOF
<CODE><B>
Registered trademarks are the property of their respective owners
</B></CODE>
EOF
;close_column_v ();
print OUT "</A>";

close_html ();

###############################################################
###
### author.html
###
###############################################################
print "author.html\n";

open_html ("author","PSXDEV authors");
make_headline("PSXDEV authors");

open_box_no_title ();
print OUT<<EOF
$author
EOF
;close_box ();

close_html ();

###############################################################
###
### psxcurvegen.html
###
###############################################################
print "psxcurvegen.html\n";

open_html ("psxcurvegen","PSXDEV Curve Generator");
make_headline("PSXDEV Curve Generator");

open_box (2,"Screen shots");
add_box_header (2,"image","description");

add_image ("curvegen");
print OUT<<EOF
This was written in one or two hours. It's just an GAMMA control widget
abused for free-form (or spline controlled) curve editing. You can
<b>paint</b> a curve, specify its range and datatype and export it as
raw data. That's all. It has an experimental feature call <b>live update</b>:
if you know the absolute address of a curve in the remote memory, you can
directly upload it into a running application!
EOF
;close_box ();

# automatically generated

close_html ();

###############################################################
###
### FAQ.html
###
###############################################################
print "FAQ.html\n";

open_html ("FAQ","PSXDEV FAQ");
make_headline("PSXDEV Frequently Asked Questions");

open_box (2,"Index of frequently asked questions");
add_box_header (2,"category","question");
my $idx = 0;
foreach $_ (`ls ~/psxdev/data/faqs/*`)
{
	open(FH,"$_");
	if (<FH> eq "<FAQ>\n")
	{
		$question = <FH>;
		$category = <FH>;
		chop ($question);
		chop ($category);
		$link = "<A HREF=faq$idx.html>$question</A>";
		add_colorized_box_entry ($category,$link);
		$idx++;
	}
	close (FH);
}
close_box ();

close_html ();

my $idx = 0;
foreach $_ (`ls ~/psxdev/data/faqs/*`)
{
	open(FH,"$_");
	if (<FH> eq "<FAQ>\n")
	{
		$question = <FH>;
		$category = <FH>;
		chop ($question);
		chop ($category);

		open_html ("faq$idx","PSXDEV FAQ - $question");
		make_headline("Question: $question");

		open_box_no_title ();
		while (<FH>)
		{
			print OUT
		}
		close_box();

		$idx++;
		close_html();
	}
	close (FH);
}

###############################################################
###
### db3d.html
###
###############################################################
print "db3d.html\n";

open_html ("db3d","Unsupported:DB3D");
make_headline("Unsupported:DB3D");

open_box_no_title ();
print OUT<<EOF
This is my private and not supported <b>object detailer</b>. It is just
a <b>file format converter</b>; it reads
<ul>
<li>3D studio (.3DS)
<li>MD2 (quake 2)
<li>OBJ (alias|wavefront, only partially)
<li>TMD (?)
<li>TRI (simple triangle meshes)
<li>psx, x3d, db[0-9] (own formats)
</ul>
and it exports only my custom formats. This utility allowed me to test
and train various software techniques, but unfortunately it is a totally
messed up source code. I added all the features I needed for my tests just
in time. Please understand that I can not answer any questions about this
software. I am working on a completely new version with more features (like
BSP tree creation, world editor, more animation support, ...).
EOF
;close_box();

open_box (2,"Screen shots");
add_box_header (2,"image","description");

;add_image_jpg ("db3d_main");
print OUT<<EOF
This is the main window of DB3D. You will always see the current object and
its current frame (if it is an animation). You can select sets of vertices
or polygones and then modify their attributes like color, position, texcoords
and so on. All that lines and numbers are control informations (like the
shading/normal vectors). It is possible to directly exchange models with a
running remote program (without restarting it :-) but that is only a
nice gimmick. You can kind of bend and scale sets of vertices, which allows
you to create simple animations by adding modified base frames. If you are
talented you could create a Quake2 like model including its animation etc.
just from a cube (by adding color, texture, and shape data), however this
tool is just a <i>converter</i>. It can simulate the PSX 3D quality.
EOF
;add_image_jpg ("db3d_objects");
print OUT<<EOF
DB3D has an object repository, but I never finished the merge/split of
objects. This is why the use of multiple objects does not make any sense
in this program.
EOF
;add_image_jpg ("db3d_textures");
print OUT<<EOF
DB3D does only support one texture per object; this is a bad restriction,
but since the input formats do so neither it does not matter...
EOF
;add_image_jpg ("db3d_viewer");
print OUT<<EOF
This is the u,v texture coordinates editor. You will see the currently
selected texture and the u,v grid. You can select and drag the coordinates
with a mouse, rotate, flip and zoom into them. Clicking on a mapper button
will map the selected vertices from 3D to 2D (sperical, planar, cylindrical).
It seems that I overmerged that last feature by accident (<i>..and now have
to search for it in a CVSROOT backup :-( </i>)
EOF
;add_image_jpg ("db3d_coords");
print OUT<<EOF
More texture functions. This is really lowlevel; but the main design work
must be done with a real 3D application. It is just to <i>debug</i> some
nasty errors (like a misplaced eye texture...)
EOF
;add_image_jpg ("db3d_bones");
print OUT<<EOF
The bone animation support is also very uncompleted. The complete IK solver
is missing; one major problem was the internal data representation of DB3D;
my classes had some design bugs - I am working on a new library now.
EOF
;add_image_jpg ("db3d_color");
print OUT<<EOF
Nothing to say, a vertex colorizer.. 
EOF
;add_image_jpg ("db3d_gimp");
print OUT<<EOF
Have I mentioned that this complete application can be run as a GIMP plug-in?
It installs itself into the GIMP and then you can <b>realtime texture paint</b>
your objects. I stole the idea from an available plug-in, just search for
TexturePaint in freshmeat.net...
EOF
;close_box ();
close_html ();

###############################################################
###
### hardware.html
###
###############################################################
print "hardware.html\n";

open_html ("hardware","About the required hardware");
make_headline("About the required hardware");

open_box_no_title ();
print OUT<<EOF
I am getting a lot of questions about the necessary and required hardware.
EOF
;close_box ();

open_box (2,"Screen shots");
add_box_header (2,"image","description");
;add_image_jpg ("pio");
print OUT<<EOF
If your PSX does not have this parallel port you will have a problem.
Newer machines do not have it anymore.
EOF
;add_image_jpg ("gamebuster");
print OUT<<EOF
This is the cartridge you will need; it has many names like GameBuster,
ActionReplay, Equalizer and so on.
EOF
;add_image_jpg ("commslink");
print OUT<<EOF
And this is the required ISA card, kind of a full 8-bit bi-directional parallel
port.
EOF
;close_box ();
close_html ();

###############################################################
###
### contest.html
###
###############################################################
print "contest.html\n";

open_html ("contest","PSXDEV logo contest");
make_headline("PSXDEV LOGO CONTEST!");

open_box_no_title ();
print OUT<<EOF
This page will hold the contestants logo distributions...
EOF
;close_box ();

open_column ("CONTEST RULES");
print OUT<<EOF
<ul>
<li>only one image per contestant
</ul>
EOF
;close_column();
close_html ();













###############################################################
###
### about.html
###
###############################################################
print "about.html\n";

open_html ("about","About myself");
make_headline("About Daniel Balster");

open_box_no_title ();
print OUT<<EOF
It's meee, maaario!.
EOF
#'
;close_box ();

close_html ();

###############################################################
###
### author.html
###
###############################################################
print "author.html\n";

open_html ("author","About authors");
make_headline("About authors");

open_box_no_title ();
print OUT<<EOF
About the authors...
EOF
;close_box ();

close_html ();

###############################################################
###
### demos.html
###
###############################################################
print "demos.html\n";

open_html ("demos","application demos");
make_headline("PSXDEV application demos");

open_box_no_title ();
print OUT<<EOF
PSXDEV target application demos will be placed here.
EOF
;close_box ();

close_html ();

###############################################################
###
### images.html
###
###############################################################
print "images.html\n";

open_html ("images","screen shots");
make_headline("PSXDEV screen shots");

open_box_no_title ();
print OUT<<EOF
Some screen shots
EOF
;close_box ();

close_html ();

###############################################################
###
### resume.html
###
###############################################################
print "resume.html\n";

open_html ("resume","CV and resume");
make_headline("My online CV and resume");

open_box_no_title ();
print OUT<<EOF
Do you need a talented guy in your team that is able
to rapid prototype and implement everything you want?
I am also very good at teaching high-tech stuff to
newbies and...
EOF
;close_box ();

close_html ();
