HUMBLE_INSTALL_DIR=~/ros2_humble/
DEB_VERSION=$(env -i bash -c '. /etc/os-release; echo $VERSION_CODENAME')
PACKAGE_NAME=darwin-legacy
INSTALL_DIR=/etc/$PACKAGE_NAME
INCLUDE_DIR=/usr/include/$PACKAGE_NAME
ROS_BUILD_FILE=do_build.sh
ROS_RUN_FILE=run_darwin_lofaro.sh
ROS_CLOCK_RUN_FILE=run_darwin_clock_lofaro.sh
BIN_NAME=darwin-lofaro
SHM_NAME=setshm.sh
SYSTEM_ACH_DIR=ctrl_ach
SYSTEM_ACH_DIR_WALKING=walking
SYSTEM_ACH_DIR_ON=on
SYSTEM_ACH_DIR_OFF=off
SYSTEM_ACH_DIR_SERVER=ctrl_server
BIN_NAME_ACH_SERVER=darwin-server
BIN_NAME_ACH_ON=darwin-on
BIN_NAME_ACH_OFF=darwin-off
BIN_NAME_ACH_WALKING=darwin-walking
BIN_NAME_DARWIN_ACH=darwin-ach
THE_INSTALL_DIR=$(pwd)
THE_ARCH=$(arch)

InstallRos2()
{
  cd $THE_INSTALL_DIR
  InstallRos2Dep
  InstallRos2SoruceIni
  CreateSwap
  InstallRos2Soruce
}

InstallRos2Dep()
{
  cd $THE_INSTALL_DIR
  locale  # check for UTF-8

  sudo apt update && sudo apt install locales
  sudo locale-gen en_US en_US.UTF-8
  sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
  export LANG=en_US.UTF-8

  locale  # verify settings

  apt-cache policy | grep universe

  sudo apt install software-properties-common
  sudo add-apt-repository universe

  sudo apt update && sudo apt install curl gnupg lsb-release
  sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg


  echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(source /etc/os-release && echo $DEB_VERSION) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

  sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    git \
    python3-flake8 \
    python3-flake8-* \
    python3-pip \
    python3-pytest \
    python3-pytest-cov \
    python3-pytest-repeat \
    python3-pytest-rerunfailures \
    python3-rosdep \
    python3-setuptools \
    python3-vcstool \
    wget

  sudo apt install libtinyxml2-dev
  sudo apt install libasio-dev 
  sudo apt install libacl1-dev
  sudo apt install gcc g++ cmake libacl1-dev libncurses5-dev pkg-config
  sudo apt install libeigen3-dev
  sudo apt install python3-lark
  sudo apt install python3-numpy
  sudo apt install libldap2-dev
  sudo apt install rtirq-init
  sudo apt install libbullet-dev
  sudo apt install python3-opencv
  sudo apt install libopencv-dev
  sudo apt-get install mesa-common-dev
  sudo apt install libqt5core5a
  sudo apt-get install qtbase5-dev
  sudo apt-get install qtdeclarative5-dev
  sudo apt install python-is-python3
  sudo pip3 install netifaces
  sudo apt install setserial
  sudo apt install python3-colcon-common-extensions


  # Remove  root access for serial port
  sudo apt remove modemmanager
#  sudo usermod -a -G dialout username

#  THE_DIR=$(pwd)
#
#  cd /tmp/
#  git clone https://github.com/eclipse-cyclonedds/cyclonedds.git
#  cd cyclonedds
#  mkdir build
#  cmake ../
#  make
#  sudo make install
#
#  cd $THE_DIR

  echo ' '
  echo '--------- Please Restart ---------'
  echo ' '

}

InstallRos2SourceIni()
{
  cd $THE_INSTALL_DIR
  mkdir -p $HUMBLE_INSTALL_DIR/src
  cp ros2.repos.32bit $HUMBLE_INSTALL_DIR/

  cd $HUMBLE_INSTALL_DIR
  #wget https://raw.githubusercontent.com/ros2/ros2/humble/ros2.repos
  vcs import src < ros2.repos.32bit

#  sudo apt upgrade

  sudo rosdep init
  rosdep update
  rosdep install --from-paths src --ignore-src -y --skip-keys "fastcdr rti-connext-dds-6.0.1 urdfdom_headers"
}


CreateSwap()
{
  cd $THE_INSTALL_DIR
  echo '-----------------------------------'
  echo '-------- Creating 4gb SWAP --------'
  echo '-----------------------------------'
## create 4gb swap
  # Create the partition path
  sudo mkdir -p /var/cache/swap/
  # Set the size of the partition
  # bs=64M is the block size, count=64 is the number of blocks, so the swap space size is bs*count=4096MB=4GB
  sudo dd if=/dev/zero of=/var/cache/swap/swap0 bs=64M count=64
  # Set permissions for this directory
  sudo chmod 0600 /var/cache/swap/swap0
  # Create the SWAP file
  sudo mkswap /var/cache/swap/swap0
  # Activate the SWAP file
  sudo swapon /var/cache/swap/swap0
  # Check if SWAP information is correct
  sudo swapon -s
  echo '-----------------------------------'
  echo '-------- 4gb SWAP Created ---------'
  echo '-----------------------------------'
}

InstallRos2Source()
{

  cd $THE_INSTALL_DIR
  cd $HUMBLE_INSTALL_DIR
  colcon build --symlink-install --packages-skip-build-finished --parallel-workers 1
  echo ". ~/ros2_humble/install/local_setup.bash > /dev/null" >> ~/.bashrc
  echo "setserial /dev/ttyUSB0 low_latency > /dev/null" >> ~/.bashrc
}

InstallCm730()
{
  cd $THE_INSTALL_DIR
  sudo systemctl mask brltty.path
  sudo systemctl mask brltty
  cd $HUMBLE_INSTALL_DIR/src
  rm -rf ros2_cm730
  git clone https://github.com/thedancomplex/ros2_cm730

  cd $HUMBLE_INSTALL_DIR
  colcon build --symlink-install --packages-skip-build-finished
}

LowLatency()
{
  	cd $THE_INSTALL_DIR
	setserial /dev/ttyUSB0 low_latency
#	sudo setserial /dev/ttyUSB0 low_latency
	sync
#	sudo sync
	sleep 1
	echo '---- Low Latency Set ----'
}

DynInstall()
{
  	cd $THE_INSTALL_DIR
	THE_DIR=$(pwd)
	cd /tmp
	git clone https://thedancomplex/DynamixelSDK.git
        cd DynamixelSDK/c++
	if [ $THE_ARCH -eq "i686" ];
	then
		cd linux32
	elif [ $THE_ARCH -eq "x86_64" ];
	then
		cd linux64
	else
		cd linux_sbc
	fi
	make
	sudo make install
	cd $THE_DIR
}

AchInstall()
{
  	cd $THE_INSTALL_DIR
	THE_DIR=$(pwd)
	cd /tmp

	sudo apt install autotools-dev
	sudo apt-get install autoconf
	sudo apt-get install autoconf automake libtool autoconf-archive
	sudo apt-get install linux-headers-generic dkms openbsd-inetd help2man man2html docbook-utils avahi-utils
	sudo apt-get install doxygen
	sudo apt install dkms
	sudo apt-get install openbsd-inetd

	git clone https://github.com/thedancomplex/ach
	cd ach
	#git checkout os/32bit
	git checkout no/benchmarks

	autoreconf -i
	./configure --with-python --enable-dkms=no
	make
	sudo make install

	echo '8076  stream  tcp  nowait  nobody  /usr/local/bin/achd /usr/local/bin/achd serve' | sudo tee -a /etc/inetd.conf
	
	sudo service openbsd-inetd restart

	sudo ln -s /usr/local/lib/libach* /usr/lib/

	cd $THE_DIR
}

DarwinLegacyRos2()
{
  	cd $THE_INSTALL_DIR
	DarwinLegacy
	THE_DIR=$(pwd)
	sudo chmod -R 777 $INSTALL_DIR/ros2
	cp ../scripts/$ROS_BUILD_FILE $INSTALL_DIR/ros2
	cp ../scripts/$ROS_RUN_FILE $INSTALL_DIR/ros2
	cp ../scripts/$ROS_CLOCK_RUN_FILE $INSTALL_DIR/ros2
#	sudo cp ../scripts/$BIN_NAME $INSTALL_DIR
	cd $INSTALL_DIR
#	sudo chmod +x $BIN_NAME
	cd $INSTALL_DIR/ros2
	chmod +x $ROS_BUILD_FILE
#	sudo rm /usr/bin/$BIN_NAME
#	sudo ln -s $INSTALL_DIR/$BIN_NAME /usr/bin
	chmod +x $ROS_RUN_FILE
	source $ROS_BUILD_FILE
	sudo chmod -R 755 $INSTALL_DIR/ros2
	
	cd $THE_DIR
}

DarwinAchOff()
{
  $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_OFF/$BIN_NAME_ACH_OFF
}

DarwinAchOn()
{
  $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_ON/$BIN_NAME_ACH_ON
}

DarwinAchServer()
{
  cd $THE_INSTALL_DIR
  case "$1" in
    	'on' )
		DarwinAchServerOn $@
	;;
    	'off' )
		DarwinAchServerOff $@
	;;
	
	* )
		ShowUsage
		exit 1
  esac

}

DarwinAchServerOn()
{
  if ps -p $(pidof $BIN_NAME_ACH_SERVER) > /dev/null 2>&1
  then
      echo 'Darwin-Ach Server is already running'
  else
      echo 'Starting Darwin-Ach Server'
      $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_SERVER/$BIN_NAME_ACH_SERVER & > /dev/null 2>&1
  fi
}

DarwinAchServerOff()
{
  if ps -p $(pidof $BIN_NAME_ACH_SERVER) > /dev/null 2>&1
  then
      echo 'Stopping Darwin-Ach Server'
      kill -9 $(pidof $BIN_NAME_ACH_SERVER) > /dev/null 2>&1
  else
      echo 'Darwin-Ach Server is already stopped'
  fi
}

DarwinAchWalking()
{
  case "$1" in
    	'on' )
		DarwinAchWalkingOn $@
	;;
    	'off' )
		DarwinAchWalkingOff $@
	;;
	
	* )
		ShowUsage
		exit 1
  esac

}

DarwinAchWalkingOn()
{
  if ps -p $(pidof $BIN_NAME_ACH_WALKING) > /dev/null 2>&1
  then
      echo 'Darwin-Ach Walking is already running'
  else
      echo 'Starting Darwin-Ach Walking'
      $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_WALKING/$BIN_NAME_ACH_WALKING & > /dev/null 2>&1
  fi
}

DarwinAchWalkingOff()
{
  if ps -p $(pidof $BIN_NAME_ACH_WALKING) > /dev/null 2>&1
  then
      echo 'Stopping Darwin-Ach Walking'
      kill -9 $(pidof $BIN_NAME_ACH_WALKING) > /dev/null 2>&1
  else
      echo 'Darwin-Ach Walking is already stopped'
  fi
}


DarwinAchInstallServer()
{
  cd $THE_INSTALL_DIR
  cd $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_WALKING
  sudo ./build.sh
  cd $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_ON
  sudo ./build.sh
  cd $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_OFF
  sudo ./build.sh
  cd $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_SERVER
  sudo ./build.sh
}

DarwinAchInstallClient()
{
  cd $THE_INSTALL_DIR
  cd $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_WALKING
  sudo ./build.sh
  cd $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_ON
  sudo ./build.sh
  cd $INSTALL_DIR/$SYSTEM_ACH_DIR/$SYSTEM_ACH_DIR_OFF
  sudo ./build.sh
}

DarwinAchInstall()
{

  cd $THE_INSTALL_DIR
  THE_DIR=$(pwd)

  case "$1" in
    	'server' )
		DynInstall
  		AchInstall
		DarwinLegacy
		DarwinAchInstallServer
	;;
    	'client' )
  		AchInstall
		DarwinLegacy
		DarwinAchInstallClient
	;;
	
	* )
		ShowUsage
		exit 1
  esac

  cd $THE_DIR

  sudo rm /usr/bin/$BIN_NAME_DARWIN_ACH
  sudo cp ../scripts/$BIN_NAME_DARWIN_ACH $INSTALL_DIR
  sudo ln -s $INSTALL_DIR/$BIN_NAME_DARWIN_ACH /usr/bin

  cd $THE_DIR
}

DynInstall()
{
  	cd $THE_INSTALL_DIR
	THE_DIR=$(pwd)
	cd /tmp
	git clone https://github.com/thedancomplex/DynamixelSDK
	cd DynamixelSDK
	cd c++/build
	if [ $THE_ARCH == "x86" ]
	then
		cd linux32
		make
		sudo make install
		cd ..
	elif [ $THE_ARCH == "x86_64" ]
	then
		cd linux64
		make
		sudo make install
		cd ..
	else
		cd linux_sbc
		make
		sudo make install
		cd ..
	fi

}

DarwinLegacy()
{
  	cd $THE_INSTALL_DIR
	THE_DIR=$(pwd)
	sudo rm -rf $INSTALL_DIR
	sudo mkdir -p $INSTALL_DIR
	echo $INSTALL_DIR
        sudo cp -r ../include/ $INSTALL_DIR/
        sudo cp -r ../ros2/ $INSTALL_DIR/
        sudo cp -r ../$SYSTEM_ACH_DIR $INSTALL_DIR/
#	sudo mkdir /etc/rc.local.d
#	chmod +x darwin-legacy.sh
#	sudo cp darwin-legacy.sh /etc/rc.local.d/
#	echo $INCLUDE_DIR
#	sudo rm $INCLUDE_DIR
#        sudo ln -s $INSTALL_DIR/include $INCLUDE_DIR
	sudo cp ../scripts/$BIN_NAME $INSTALL_DIR
        sudo mkdir -p /etc/rc.local.d/
        sudo cp ../scripts/$SHM_NAME $INSTALL_DIR
	cd $INSTALL_DIR
	sudo chmod +x $BIN_NAME
	sudo chmod +x $SHM_NAME
	sudo rm /usr/bin/$BIN_NAME
	sudo rm /etc/rc.local.d/$SHM_NAME
	sudo ln -s $INSTALL_DIR/$BIN_NAME /usr/bin
	sudo ln -s $INSTALL_DIR/$SHM_NAME /etc/rc.local.d/
	cd $THE_DIR
}

ShowUsage()
{
	echo 
	echo '================================================='
	echo '================================================='
	echo '============= ROS2 and CM730 Install============='
	echo '=============  for the Darwin OP    ============='
	echo '================================================='
	echo '=============== Daniel M. Lofaro ================'
	echo '=============== dan@danlofaro.com ==============='
	echo '================================================='
	echo '================================================='
	echo ''
	echo 'ros2          : installs ros2 Dep and ros2 from  '
        echo '                soruce (~24hr on Darwin OPs CPU) '
        echo '                       (~20hr on raspi 3b+ CPU)  '
	echo 'ros2-dep      : installs ros2 dep                '
	echo 'ros2-src-ini  : initialize soruce                '
	echo 'ros2-src      : installs ros2 from source        '
        echo '                 (~24hr on Darwin OPs CPU)       '
        echo '                 (~20hr on raspi 3b+ CPU)        '
	echo 'swap          : Create 4gb swap                  '
	echo 'cm730         : installs cm730 (ros2) drivers    '
	echo 'low-latency   : sets serial to low latency mode  '
        echo 'darwin-legacy : install the darwin-legacy system '
        echo 'darwin-ros2   : install the darwin-ros2 system   '
	echo ''
	echo 'darwin-ach                                       '
	echo '    server    : installs darwin-ach server       '
	echo '                Use on the Darwins cpu (fit-pc)  '
	echo '    client    : installs darwin-ach client       '
	echo '                Use on external computer/backpack'
	echo
}


case "$1" in
	'ros2' )
		InstallRos2 $@
	;;

	'ros2-src' )
		InstallRos2Source $@
	;;
	'ros2-src' )
		InstallRos2SourceIni $@
	;;

	'swap' )
		CreateSwap $@
	;;

	'ros2-dep' )
		InstallRos2Dep $@
	;;
	
	'cm730' )
		InstallCm730 $@
	;;
        'low-latency' )
		LowLatency $@
	;;

	'darwin-legacy' )
		DarwinLegacy $@
	;;

	'darwin-ros2' )
		DarwinLegacyRos2 $@
	;;
     
        'darwin-ach' )
		DarwinAchInstall $2
	;;

	* )
		ShowUsage
		exit 1
esac

exit 0



