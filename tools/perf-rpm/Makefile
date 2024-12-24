# Makefile to build perf and package it into a rpm file.

all:
	rpmbuild --define '_topdir $(CURDIR)' -ba $(CURDIR)/SPECS/perf.spec

clean:
	rm -rf $(CURDIR)/BUILD
	rm -rf $(CURDIR)/BUILDROOT
	rm -rf $(CURDIR)/RPMS
	rm -rf $(CURDIR)/SOURCES
	rm -rf $(CURDIR)/SRPMS
