# pswd-k1ll

## WIP

pswd-k1ll is a passwd shim which should steal the passwords when changed.

pswd-k1ll functions as /bin/passwd but also sends the passwd to the listener

## To run

Compile pswd-k1ll with ```gcc```

Drop pswd-k1ll on the target

Move or remove the passwd binary and copy pswd-k1ll in is place. 

Give pswd-kill 755 perms and an suid bit

change the owner to root

and BOOM now when a user runs passwd, pswd-k1ll is run instead

Make sure to keep the listener running 

Happy Hacking

```ps please use this only where you have authorized access```

```ps 2 rn it will lock any users that use it, most likely because of the encryption```

im working on it^

also going to work on: running the listener as a service, a ui for that service that runs on nginx, making pswd-k1ll a ansible template to drop anywhere given the listener ip, and using ansible to automatically configure pswd-k1ll