<?php
    $user = explode("=", $_SERVER['QUERY_STRING']);
    echo 'Hello '  . $user[1] . '! (I got your name from the URL ;)';
?>