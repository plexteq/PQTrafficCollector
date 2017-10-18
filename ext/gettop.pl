#!/usr/bin/perl -w

use strict;
use warnings;
use Getopt::Long;
use DBI;
use Socket qw( inet_aton inet_ntoa );
use POSIX qw(strftime isdigit);
use Math::Round;
require Time::HiRes;

my $file = strftime("/var/db/pqtc/ts-%m-%Y.sl3", localtime());
my $top = 3;
my $min = 180;

sub LongToDottedQuad {
 return inet_ntoa(pack('N', shift)); 
}

GetOptions(
    "f|file=s" => \$file,
    "m|min=i" => \$min,
    "t|top=i"   => \$top
);

my %protos = (0, 'TCP', 1, 'UDP', 2, 'ICMP', 3, 'OTHER');

my $dsn      = "dbi:SQLite:dbname=$file";
my $user     = "";
my $password = "";
my $dbh = DBI->connect($dsn, $user, $password, {
   PrintError       => 0,
   RaiseError       => 1,
   AutoCommit       => 1,
   FetchHashKeyName => 'NAME_lc',
});

print "Stats for top $top IPs, for last $min sec. from $file \n";

my $start_ts = Time::HiRes::time();

my $sql_top_ipaddr = "SELECT ip, (SUM(tcp_in_bytes + udp_in_bytes + tcp_out_bytes) + ".
	  "SUM(udp_out_bytes + icmp_in_bytes + icmp_out_bytes)+ ".
	  "SUM(other_in_bytes) + SUM(other_out_bytes)) / (1024*1024) traf ".
	  "FROM totals WHERE ts BETWEEN ? AND ? GROUP BY ip ORDER BY traf DESC LIMIT ?";
my $sth_top_ipaddr = $dbh->prepare($sql_top_ipaddr);

$sth_top_ipaddr->execute(time() - $min, time(), $top);
#print $sql_top_ipaddr . ", " . (time() - $min) . ", " . time() . ", " . $top . "\n";

while (my @row_top = $sth_top_ipaddr->fetchrow_array)
{
	my $mbps = sprintf("%.2f", $row_top[1] / $min);
	my $mibps = sprintf("%.2f", ($row_top[1] / $min) * 8);
	
	print " \n" . LongToDottedQuad($row_top[0]) . " total: " . $row_top[1] . "MB, rate: $mbps MB/s ($mibps Mbit/s)\n";

	my $sql_traftype =
			"SELECT ptype, SUM(in_bytes)/(1024*1024), SUM(out_bytes)/(1024*1024) ".
			"FROM protos WHERE ip=? AND ts BETWEEN ? AND ? AND ptype=0 ".
			"UNION ".
			"SELECT ptype, SUM(in_bytes)/(1024*1024), SUM(out_bytes)/(1024*1024) ".
			"FROM protos WHERE ip=? AND ts BETWEEN ? AND ? AND ptype=1";

	my $sth_traftype = $dbh->prepare($sql_traftype);
	$sth_traftype->execute($row_top[0], time() - $min, time(), $row_top[0], time() - $min, time());

	#print $sql_traftype . "   " . $row_top[0] . ", " . (time() - $min) . ", " . time() . "\n";

	while (my @row_type = $sth_traftype->fetchrow_array)
	{
		next unless (defined($row_type[0]));

		print $protos{$row_type[0]} . " -> in: " . $row_type[1] . "MB, out: " . $row_type[2] . " MB\n";

		my $sql_ports = "SELECT port, SUM(in_bytes+out_bytes)/(1024*1024) s ".
				"FROM protos WHERE ip=? AND ptype=? AND ts BETWEEN ? AND ? GROUP BY port HAVING s > 0 ORDER BY s DESC LIMIT 5";
		my $sth_ports = $dbh->prepare($sql_ports);
		$sth_ports->execute($row_top[0], $row_type[0], time() - $min, time());

		#print "$sql_ports  " . $row_top[0] . ", " . $row_type[0] . ", " . (time() - $min) . ", " . time() . "\n";

		while (my @row_ports = $sth_ports->fetchrow_array) 
		{
			print "  [" . $row_ports[0] . "] total: " . $row_ports[1] . "MB \n";
		}
	}
}

$dbh->disconnect;

my $exec_time = sprintf("%.4f", Time::HiRes::time() - $start_ts);

print " \n";
print "$exec_time ms.\n";
