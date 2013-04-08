Summary: A C library for handling JSON data structures.
Name: jsondata
Provides: jsondata
Version: 0.03
Release: 1
License: GPL
Group: Applications/Multimedia/
Source0: %{name}-%{version}.tar.gz
URL: https://github.com/AndyA/jsondata
Vendor: Andy Armstrong <andy@hexten.net>
Packager: Andy Armstrong <andy@hexten.net>
BuildArch: i386 x86_64
BuildRoot: %{_builddir}/%{name}-root
#Requires: 
#BuildRequires: 

%description
A C library for handling JSON data structures.

%prep
%setup -n %{name}-%{version}

%build
./configure --prefix=/usr
make

%install
make DESTDIR=%{buildroot} install

%clean
rm -rf %{buildroot}

%files
/usr/bin/jsonpretty
/usr/include/jd_pretty.h
/usr/include/jsondata.h
/usr/lib/libjsondata.la
/usr/lib/libjsondata.so
/usr/lib/libjsondata.a
