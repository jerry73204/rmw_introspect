#!/bin/bash
# Install RMW implementation dependencies for testing rmw_introspect
# Supports Ubuntu 22.04 with ROS 2 Humble

set -e

echo "==================================================================="
echo "Installing RMW Implementation Dependencies for rmw_introspect"
echo "==================================================================="
echo ""

# Check if ROS 2 is installed
if [ ! -f /opt/ros/humble/setup.bash ]; then
    echo "ERROR: ROS 2 Humble not found. Please install ROS 2 Humble first."
    echo "Visit: https://docs.ros.org/en/humble/Installation.html"
    exit 1
fi

echo "✓ ROS 2 Humble detected"
echo ""

# Ask about Connext DDS installation
echo "-------------------------------------------------------------------"
echo "Connext DDS Installation"
echo "-------------------------------------------------------------------"
echo ""
echo "RTI Connext DDS is a commercial DDS implementation that requires"
echo "a license. A free evaluation license is available from RTI."
echo ""
echo "Note: ros-humble-rmw-connextdds may not be available in standard"
echo "      apt repositories and typically requires manual installation."
echo ""
read -p "Do you want to attempt installation of ros-humble-rmw-connextdds? (y/N) " -n 1 -r
echo ""
INSTALL_CONNEXT=$REPLY

echo ""
echo "-------------------------------------------------------------------"
echo ""

# Source ROS 2
source /opt/ros/humble/setup.bash

echo "Installing RMW implementation packages..."
echo ""

# Array of RMW implementations to install
declare -a RMW_PACKAGES=(
    "ros-humble-rmw-fastrtps-cpp"
    "ros-humble-rmw-fastrtps-dynamic-cpp"
    "ros-humble-rmw-cyclonedds-cpp"
)

# Add Connext if user requested it
if [[ $INSTALL_CONNEXT =~ ^[Yy]$ ]]; then
    RMW_PACKAGES+=("ros-humble-rmw-connextdds")
fi

# Install each package
for package in "${RMW_PACKAGES[@]}"; do
    echo "Installing $package..."

    # Special handling for Connext (may not be available)
    if [[ $package == "ros-humble-rmw-connextdds" ]]; then
        if sudo apt-get install -y "$package" 2>&1 | grep -q "Unable to locate package"; then
            echo "  ⚠ $package not available in apt repositories"
            echo "  Please install manually from:"
            echo "    https://docs.ros.org/en/humble/Installation/DDS-Implementations/Working-with-RTI-Connext-DDS.html"
            continue
        fi
    else
        sudo apt-get install -y "$package"
    fi
done

echo ""
echo "Checking for rmw_zenoh_cpp..."
# rmw_zenoh is not in the standard Humble repos yet, check if available
if apt-cache search ros-humble-rmw-zenoh-cpp | grep -q "ros-humble-rmw-zenoh-cpp"; then
    echo "  Installing ros-humble-rmw-zenoh-cpp..."
    sudo apt-get install -y ros-humble-rmw-zenoh-cpp
else
    echo "  ⚠ ros-humble-rmw-zenoh-cpp not available in apt repositories"
    echo "  You can build it from source if needed:"
    echo "    https://github.com/ros2/rmw_zenoh"
fi

echo ""
echo "Installing additional test dependencies..."
sudo apt-get install -y \
    ros-humble-test-msgs \
    ros-humble-std-msgs \
    ros-humble-std-srvs \
    valgrind

echo ""
echo "==================================================================="
echo "✓ Installation complete!"
echo "==================================================================="
echo ""
echo "Installed RMW implementations:"
echo ""

# List installed RMW packages
dpkg -l | grep "ros-humble-rmw-" | awk '{print "  - " $2 " (" $3 ")"}'

echo ""
echo "To use a specific RMW implementation, set the RMW_IMPLEMENTATION"
echo "environment variable before running your ROS 2 nodes:"
echo ""
echo "  export RMW_IMPLEMENTATION=rmw_fastrtps_cpp"
echo "  export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp"
echo "  export RMW_IMPLEMENTATION=rmw_zenoh_cpp"
if [[ $INSTALL_CONNEXT =~ ^[Yy]$ ]]; then
    echo "  export RMW_IMPLEMENTATION=rmw_connextdds  # (if installed)"
fi
echo ""
echo "For rmw_introspect intermediate mode testing, use:"
echo ""
echo "  export RMW_IMPLEMENTATION=rmw_introspect_cpp"
echo "  export RMW_INTROSPECT_DELEGATE_TO=rmw_fastrtps_cpp"
echo ""
if [[ $INSTALL_CONNEXT =~ ^[Yy]$ ]]; then
    echo "Note: If Connext DDS was not installed via apt, you may need to"
    echo "      install it manually and set up the license. See:"
    echo "      https://docs.ros.org/en/humble/Installation/DDS-Implementations/Working-with-RTI-Connext-DDS.html"
    echo ""
fi
echo "See docs/TESTING.md for more details."
echo ""
