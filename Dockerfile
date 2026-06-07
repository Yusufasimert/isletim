FROM contiker/contiki-ng:latest

# Switch to root for system-level operations
USER root

# Install Xvfb (virtual framebuffer) so Cooja can run without a physical X11 display
RUN apt-get update -qq && apt-get install -y -qq xvfb > /dev/null 2>&1 && rm -rf /var/lib/apt/lists/*

# Remove the default template directory in the base image
RUN rm -rf /home/user/contiki-ng

# Copy our workspace contiki-ng directory (which includes our customized examples/BIL304-OTA folder)
# and assign ownership to the 'user' account in the container
COPY --chown=user:user contiki-ng /home/user/contiki-ng

# Switch to 'user' to run the compilation
# This ensures that built files inside the container are owned by the non-root user
USER user
WORKDIR /home/user/contiki-ng

# Pre-compile the OTA server and client applications for the MSP430-based z1 target
RUN cd examples/BIL304-OTA && make TARGET=z1

# Switch back to root so that the inherited entrypoint script (/usr/local/bin/remap-user.sh)
# can run with root privileges to adjust UID/GID mapping on startup
USER root
