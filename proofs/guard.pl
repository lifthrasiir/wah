#!/usr/bin/perl
# guard.pl SECS MAX_MB cmd...: runs cmd, killing it beyond SECS seconds or MAX_MB megabytes of RSS (polled every
# second, since macOS has no limit on it), and reports the time and the peak RSS on stderr
use strict; use POSIX ":sys_wait_h"; use Time::HiRes qw(sleep time);
my ($secs, $mb, @cmd) = @ARGV;
my $pid = fork(); if (!$pid) { exec @cmd or die; }
my ($t0, $peak, $why) = (time, 0, "");
while (1) {
    my $r = waitpid($pid, WNOHANG);
    if ($r == $pid) { my $st = $? >> 8; printf STDERR "guard: exit %d, %.0fs, peak %d MB\n", $st, time - $t0, $peak; exit $st; }
    my $rss = `ps -o rss= -p $pid`; $rss =~ s/\s//g; $rss = int(($rss || 0) / 1024);
    $peak = $rss if $rss > $peak;
    $why = "memory" if $rss > $mb; $why = "time" if time - $t0 > $secs;
    if ($why) { kill 9, $pid; waitpid($pid, 0); printf STDERR "guard: killed for %s, %.0fs, peak %d MB\n", $why, time - $t0, $peak; exit 142; }
    sleep 1;
}
