#!/usr/bin/perl

#
# perl what's up - Monitors, via icmp, the server responses and send a mail
# reporting the results.
#
#
# Gleicon S. Moraes - gleicon@terra.com.br
# I using this script in the crontab, */30 * * * * 
# You can use it to alert you when a server goes down 
#
# start global config
#

# mail server config

$server_addr='mail.yournet'; 	# your SMTP server addr
$server_helo='mailservername';	# its name


# mail body config

$mail_alarm='you@yournet';			# mail addr that should receive the notification
$mail_name=$mail_alarm; 			# mail to appear in msg body
$mail_message='Server offline :';		# message
$mail_from='warning@yournet';			# accout to appear as Sender


#
# server list file format:
# each line must contain a server name or ip
#
# e.g.:
# 127.0.0.1
# www.com
# www.yahoo.com

$server_list='/etc/lista.conf';

#
# end global config
#

#
# requires Net::Ping and Net::SMTP
#
 
use Net::Ping;
use Net::SMTP;

my $servers_ok=0;
my $servers_down=0;

open (INFILE, "<$server_list");
@tudo=<INFILE>;
close(INFILE);

$p=Net::Ping->new("icmp");
foreach $lina (@tudo) {
	chomp ($lina);
	if ($p->ping($lina, 2)) {
		
		$servers_ok++;
		$servers_ok_txt=$servers_ok_txt."\n".$lina;
		
		} else {
		
		$servers_down++;
		$servers_down_txt=$servers_down_txt."\n".$lina;
#		mail_to($lina." is down", $lina." did not response", $mail_message);
		
		}
	}
	
$final_stat="\n\nServer(s) up: ".$servers_ok_txt."\n\nServer(s) down: ".$servers_down_txt."\n";


mail_to ("General stats", "list done", $final_stat);

	
$p->close();

#
# mail_to (ip, message)
#

sub mail_to {
	local ($wot, $message, $body) = @_;
	my $data=`date`;
	$smtp = Net::SMTP->new ($server_addr,
				Hello => $server_helo,
				Timeout => 20);

	$smtp->mail ($mail_from);
	$smtp->to ($mail_alarm);
	$smtp->data ();
	$smtp->datasend ("To: ".$mail_name."\n");
	$smtp->datasend ("Subject: ".$data." ".$wot." \n");
	$smtp->datasend ("\n");
	$smtp->datasend ($body. "\n". $message . "\n");
	$smtp->dataend ();

	$smtp->quit;

	}


