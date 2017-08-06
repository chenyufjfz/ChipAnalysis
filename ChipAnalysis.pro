TEMPLATE = subdirs

SUBDIRS += \
    Raknet \    
    Mdb \
    Try \
    DatServer \
    ChipExtract

#Try.depends = Raknet Mdb
#DatServer.depends = Raknet Mdb
#ChipExtract.depends = Raknet Mdb
