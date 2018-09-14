#this may work, last tested on ns3.28
#run this where you want ns3
hg clone http://code.nsnam.org/ns-3-allinone
cd ns-3-allinone && ./download.py

git clone https://github.com/jlmathew/Fate.git
git clone https://github.com/jlmathew/Fate-Ns3.git

cd ns-3-dev/src
./create-module.py fate
#update FATE
cd fate/model
ln -s  ../../../../Fate/* .
mkdir ns3
cd ns3
ln -s ../../../../../Fate-Ns3/Fate-Ns3-Interface/* .

cd ../../helper
ln -s ../../../../../Fate-Ns3/Fate-Ns3-Support/* .

cd ../../


#update applications
cd ../../applications/model/
ln -s ../../../../Fate-Ns3/Fate-Ns3-Applications/model/* .
cd ../helper
ln -s ../../../../Fate-Ns3/Fate-Ns3-Applications/helper/* .

#update waf files
cd ../../../../Fate/
ls  -p -1 *.h | grep -v / > /tmp/fateh.tmp
sed -ie "s/^/        'model\//" /tmp/fateh.tmp 
sed -ie "s/$/',/" /tmp/fateh.tmp

ls  -p -1 *.cc | grep -v / > /tmp/fatec.tmp
sed -ie "s/^/        'model\//" /tmp/fatec.tmp 
sed -ie "s/$/',/" /tmp/fatec.tmp

cd ../Fate-Ns3/Fate-Ns3-Interface
ls -p -1 *.h | grep -v / > /tmp/interfh.tmp
sed -ie "s/^/        'model\/ns3\//" /tmp/interfh.tmp 
sed -ie "s/$/',/" /tmp/interfh.tmp

ls -p -1 *.cc | grep -v / > /tmp/interfc.tmp
sed -ie "s/^/        'model\/ns3\//" /tmp/interfc.tmp 
sed -ie "s/$/',/" /tmp/interfc.tmp

cd ../Fate-Ns3-Support/helper/
ls -p -1 *.h | grep -v / > /tmp/helperh.tmp
sed -ie "s/^/        'helper\//" /tmp/helperh.tmp 
sed -ie "s/$/',/" /tmp/helperh.tmp
ls -p -1 *.cc | grep -v / > /tmp/helperc.tmp
sed -ie "s/^/        'helper\//" /tmp/helperc.tmp 
sed -ie "s/$/',/" /tmp/helperc.tmp

cat /tmp/fateh.tmp /tmp/interfh.tmp /tmp/helperh.tmp > /tmp/fate-wafh.tmp
cat /tmp/fatec.tmp /tmp/interfc.tmp /tmp/helperc.tmp > /tmp/fate-wafc.tmp


#application
cd ../../Fate-Ns3-Applications/model
ls -p -1 *.h | grep -v / > /tmp/apph.tmp
sed -ie "s/^/        'model\//" /tmp/apph.tmp 
sed -ie "s/$/',/" /tmp/apph.tmp
ls -p -1 *.cc | grep -v / > /tmp/appc.tmp
sed -ie "s/^/        'model\//" /tmp/appc.tmp 
sed -ie "s/$/',/" /tmp/appc.tmp

cd ../helper
ls -p -1 *.h | grep -v / > /tmp/app-helph.tmp
sed -ie "s/^/        'helper\//" /tmp/app-helph.tmp 
sed -ie "s/$/',/" /tmp/app-helph.tmp
ls -p -1 *.cc | grep -v / > /tmp/app-helpc.tmp
sed -ie "s/^/        'helper\//" /tmp/app-helpc.tmp 
sed -ie "s/$/',/" /tmp/app-helpc.tmp

cat /tmp/apph.tmp /tmp/app-helph.tmp > /tmp/fate-apph.tmp
cat /tmp/appc.tmp /tmp/app-helpc.tmp > /tmp/fate-appc.tmp

cd ../../../ns-3-dev/src/fate/
cp /tmp/fate-wafc.tmp tmp
sed -i.bck "/fate.cc',/r tmp" wscript
cp /tmp/fate-wafh.tmp tmp
sed -i "/fate.h',/r tmp" wscript
rm tmp
sed -i "s/'core'/'core','internet'/" wscript

cd ../applications
cp /tmp/fate-appc.tmp tmp
sed -i.bck "/bulk-send-application.cc',/r tmp" wscript
cp /tmp/fate-apph.tmp tmp
sed -i "/bulk-send-application.h',/r tmp" wscript
rm tmp
sed -i "s/'internet'/'internet','fate'/" wscript


#modify ns3 code: network.cc, 
cd ../network/model
patch node.h < ../../../../Fate-Ns3/Ns3-Modified/node.h.3.28.patch
patch node.cc < ../../../../Fate-Ns3/Ns3-Modified/node.cc.3.28.patch

#add scratch files

#add xml files!

# rerun waf

./waf configure --enable-examples --enable-mpi --enable-tests
#ls -1 ../../../Fate-Ns3/Fate-Ns3-Applications/model/* |  sed 's#.*/##' 
