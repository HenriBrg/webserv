FROM nginx:latest
# Image officielle basée sur debian:buster
# COPY index.html /usr/share/nginx/html/index.html
COPY ./nginx.conf /etc/nginx/nginx.conf


# Tips & Tricks


# --> DL des packages dans debian:buster

    # apt-get update
    # apt-get install <package>
    # exemple : apt-get install vim

# --> Infos sur l'OS hôte

    # cat /proc/version
    # cat /etc/os-release
    # uname -r

# --> Modifier le dockerfile n'implique pas un rebuilt, sauf si un ADD statement est ajouté

    # Docker build support caching as long as there is no ADD instruction.
    # If you are actively developing and changing files, only what is after the ADD will be rebuilt.

