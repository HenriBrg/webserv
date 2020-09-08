FROM nginx:latest

# docker build -t nginxwbs .
# docker run -it --rm -d -p 8080:80 --name webserver nginxwbs
# docker exec -it xxx /bin/sh

# apt-get update
# installer vim
# 
 
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

