Name:		perf
Version:	3.19.8
Release:	solos52%{?dist}
Summary:    Performance analysis tools for Linux.	

Group:		Applications/Performance
License:	N/A
URL:		https://perf.wiki.kernel.org/index.php/Main_Page
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires: 	bash
Requires:	bash

%description
Performance counters for Linux are a new kernel-based subsystem
that provide a framework for all things performance analysis. It
covers hardware level (CPU/PMU, Performance Monitoring Unit) features
and software features (software counters, tracepoints) as well

%prep
#%setup -q -n %{name}-%{version}

%build
rm -rf %{buildroot}
mkdir -p %{buildroot}
pushd %{_topdir}/../perf
make prefix=%{buildroot}/usr install
popd

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
#%attr(755,root,root) %{_bindir}/perf
%attr(755,root,root) /usr/bin/perf
%attr(755,root,root) /usr/bin/perf-read-vdso32
%attr(755,root,root) /usr/bin/trace
/usr/etc/bash_completion.d/perf
/usr/lib/traceevent/plugins/plugin_cfg80211.so
/usr/lib/traceevent/plugins/plugin_function.so
/usr/lib/traceevent/plugins/plugin_hrtimer.so
/usr/lib/traceevent/plugins/plugin_jbd2.so
/usr/lib/traceevent/plugins/plugin_kmem.so
/usr/lib/traceevent/plugins/plugin_kvm.so
/usr/lib/traceevent/plugins/plugin_mac80211.so
/usr/lib/traceevent/plugins/plugin_sched_switch.so
/usr/lib/traceevent/plugins/plugin_scsi.so
/usr/lib/traceevent/plugins/plugin_xen.so
/usr/lib64/libperf-gtk.so
%attr(755,root,root) /usr/libexec/perf-core/perf-archive
%attr(755,root,root) /usr/libexec/perf-core/perf-with-kcore
/usr/libexec/perf-core/scripts/perl/Perf-Trace-Util/lib/Perf/Trace/Context.pm
/usr/libexec/perf-core/scripts/perl/Perf-Trace-Util/lib/Perf/Trace/Core.pm
/usr/libexec/perf-core/scripts/perl/Perf-Trace-Util/lib/Perf/Trace/Util.pm
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/check-perf-trace-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/failed-syscalls-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/failed-syscalls-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/rw-by-file-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/rw-by-file-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/rw-by-pid-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/rw-by-pid-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/rwtop-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/rwtop-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/wakeup-latency-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/perl/bin/wakeup-latency-report
/usr/libexec/perf-core/scripts/perl/check-perf-trace.pl
/usr/libexec/perf-core/scripts/perl/failed-syscalls.pl
/usr/libexec/perf-core/scripts/perl/rw-by-file.pl
/usr/libexec/perf-core/scripts/perl/rw-by-pid.pl
/usr/libexec/perf-core/scripts/perl/rwtop.pl
/usr/libexec/perf-core/scripts/perl/wakeup-latency.pl
/usr/libexec/perf-core/scripts/python/Perf-Trace-Util/lib/Perf/Trace/Core.py
/usr/libexec/perf-core/scripts/python/Perf-Trace-Util/lib/Perf/Trace/EventClass.py
/usr/libexec/perf-core/scripts/python/Perf-Trace-Util/lib/Perf/Trace/SchedGui.py
/usr/libexec/perf-core/scripts/python/Perf-Trace-Util/lib/Perf/Trace/Util.py
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/event_analyzing_sample-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/event_analyzing_sample-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/export-to-postgresql-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/export-to-postgresql-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/failed-syscalls-by-pid-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/failed-syscalls-by-pid-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/futex-contention-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/futex-contention-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/net_dropmonitor-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/net_dropmonitor-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/netdev-times-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/netdev-times-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/sched-migration-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/sched-migration-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/sctop-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/sctop-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/syscall-counts-by-pid-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/syscall-counts-by-pid-report
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/syscall-counts-record
%attr(755,root,root) /usr/libexec/perf-core/scripts/python/bin/syscall-counts-report
/usr/libexec/perf-core/scripts/python/check-perf-trace.py
/usr/libexec/perf-core/scripts/python/event_analyzing_sample.py
/usr/libexec/perf-core/scripts/python/export-to-postgresql.py
/usr/libexec/perf-core/scripts/python/failed-syscalls-by-pid.py
/usr/libexec/perf-core/scripts/python/futex-contention.py
/usr/libexec/perf-core/scripts/python/net_dropmonitor.py
/usr/libexec/perf-core/scripts/python/netdev-times.py
/usr/libexec/perf-core/scripts/python/sched-migration.py
/usr/libexec/perf-core/scripts/python/sctop.py
/usr/libexec/perf-core/scripts/python/syscall-counts-by-pid.py
/usr/libexec/perf-core/scripts/python/syscall-counts.py
/usr/libexec/perf-core/tests/attr.py
/usr/libexec/perf-core/tests/attr/README
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/base-record
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/base-stat
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-C0
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-basic
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-any
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-filter-any
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-filter-any_call
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-filter-any_ret
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-filter-hv
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-filter-ind_call
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-filter-k
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-branch-filter-u
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-count
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-data
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-freq
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-graph-default
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-graph-dwarf
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-graph-fp
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-group
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-group-sampling
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-group1
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-no-delay
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-no-inherit
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-no-samples
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-period
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-record-raw
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-C0
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-basic
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-default
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-detailed-1
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-detailed-2
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-detailed-3
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-group
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-group1
%attr(755,root,root) /usr/libexec/perf-core/tests/attr/test-stat-no-inherit
