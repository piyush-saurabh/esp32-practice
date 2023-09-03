# Define global arguments
ARG UID=1000
ARG USER=roguesecurity

# Ref: https://hub.docker.com/r/espressif/idf/tags
FROM espressif/idf:v4.3.2

# Import global arguments
ARG UID
ARG USER

# Define local arguments

# Create Non-Root User
RUN ["dash", "-c", "\
    addgroup \
     --gid ${UID} \
     \"${USER}\" \
 && adduser \
     --disabled-password \
     --gecos \"\" \
     --ingroup \"${USER}\" \
     --uid ${UID} \
     \"${USER}\" \
 && usermod \
     --append \
     --groups \"dialout,plugdev\" \
     \"${USER}\" \
"]
ENV PATH="/home/${USER}/.local/bin:${PATH}"

# Install IDF as non-root user
WORKDIR /home/${USER}/
USER ${USER}

# Step 5. Update development environment
ENV LC_ALL=en_US.UTF-8