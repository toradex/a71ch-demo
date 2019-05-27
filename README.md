# A71CH Secure Element Demo

This is a demonstration of the A71CH Secure Element of NxP in combination with Toradex's Colibri iMX6ULL and Torizon. This demonstration includes a secure client - server communication, including authentication of the client, for a secure and integer file download. The given use case shows how a docker container can be exchanged with a file from a server, using implemented secure file download.

- Use of the NxP A71CH Secure Element
- Combination with the Colibri iMX6ULL
- Combination with Torizon

## Features

- Secure communication between a python webserver providing files and a client (c-application)
- Including an authorization of the client before the download of the file starts
- The client uses the A71CH Secure Element for the authentication of the server and the authorization of himself
- A shell-script is handling the full update process
  - Secure download of an updated docker container
  - Import and execution of this new docker container

When will the update fail?

- If the server doesn't have the proper certificates
- If the client doesn't have the proper certificates (equal to having the wrong A71CH connected)
- If the client doesn't have any certificates (equal to having no A71CH connected)
- If the download returned a corrupt file (wrong SHA256)
- If the server doesn't have the requested file
- If the import of the docker image failed

## Preperation

### Download of the NXP Host Software Package

Go to NXP's A71CH Secure Element product page and download the Host Software Package from "Tools & Software". Distribute the files of the corresponding folders in the related folders in your local repository (doc, hostLib and linux).

### Building of the libraries and the A71CH tools

Go to `linux/` and enable the proper toolchain for your target by enter `source <toolchain>` (e.g. source /usr/local/oecore-x86_64/environment-setup-armv7at2hf-neon-angstrom-linux-gnueabi).

First you can build the configuration tool for the A71CH called `a71chConfig_i2c_imx` by entering following line: `make -f Makefile_A71CH default app=A71CH_CONFIG conn=i2c platf=imx`. This should work without problem.

Second we want to build the library `libA71CH_i2c_imx.so`. Enter follwing line: `make -f Makefile_A71CH lib app=A71CH_LIB conn=i2c platf=imx`.

Third we want to build the specific OpenSSL engine with the follwing command: `make -f Makefile_A71CH engine app=A71CH_ENGINE conn=i2c platf=imx`.

### Creating the certificates

NxP provides a script in their host library to the A71CH Secure Element module. You can either edit the script and change the properties of the X.509 certificate which should be generated or take the default values and run it directly. The script is located under `hostLib/embSeEngine/a71chDemo/scripts/tlsCreateCredentialsRunOnClientOnce.sh`

After creating the certificates, you need to copy them to the proper location.  
Following files from `hostLib/embSeEngine/a71chDemo/ecc` need to be copied to `toradex/certs/`:

- tls_rootca.cer
- tls_server.cer
- tls_server_key.pem
- tls_client_key_pub.pem

Following files from `hostLib/embSeEngine/a71chDemo/ecc` need to be copied to the client to `~/eccKeys/`:

- tls_rootca.cer
- tls_client_key.pem
- tls_client_key_pub.pem
- tls_client.cer

### Provisioning of the A71CH Secure Element

For the provisioning of the A71CH Secure Element, the configuration application (`a71chConfig_i2c_imx`) which we compiled in chapter [Building of the libraries and the A71CH tools](#building-of-the-libraries-and-the-a71ch-tools) is required. Copy this tool to the target module.

NxP provides a script to do the provisioning of the A71CH Secure Element. Copy the script located under `hostLib/embSeEngine/a71chDemo/scripts/tlsPrepareClient.sh` to the target module. Check the references to the cert-files in the shell-script and change them to whereever you stored your cert-files. The path of `client_key_ref` points to the location where the reference key should be stored in future and the file itself does not exist yet (will be created with the provisioning). Please also check the location of the `probeExec` which should point to the copied `a71chConfig_i2c_imx`.

For the provisioning of the A71CH Secure Element, you should connect the A71CH properly and ensure that the module is in Debug Mode. If the module is not in debug mode, you could use the config-utility (Chapter XX) to configure it. If everything is okay, run the script.

### Install OpenSSL engine on target

Copy the following files compiled in the chapter [Building of the libraries and the A71CH tools](#building-of-the-libraries-and-the-a71ch-tools) to the target under `/usr/lib/`:

- libA71CH_i2c_imx.so
- libA71CH_i2c_imx.so.1 (or link the above one)
- libe2a71chi2c.so
- libe2a71chi2c.so.1.0.0 (or link the above one)

Also copy the OpenSSL configuration to the target. The configuration from the A71CH is located under `/hostLib/embSeEngine/info/opensslA71CH_i2c.cnf` and needs to be copied to the target under `/etc/ssl/`.  
_There is somehow also a ssl folder under `/usr/etc/ssl/`. To be sure, also copy it there. This is may something special from Torizon._

### Building the a71tdx application (Demo application)

The demo application consists on the target side of a C-application which is controlling and executing the secure download of a file from the server and a shell-script, which is asking for parameters, starting the C-application and importing the file into docker. The application files for the target are located in `toradex/target/`. To build the C-application, introduce the proper toolchain like in chapter [Building of the libraries and the A71CH tools](#building-of-the-libraries-and-the-a71ch-tools) and run `make`.

Copy the `a71chtdx` and the `update.sh` to the target.

## Running the demo

The demo consists of a server and a client part. If you did the [preperation](#preperation), you should be able to run the demo now.

### Starting the server part

To start the python webserver, just head to `toradex/webserver` and start the webserver with `python3 webserver.py`. All files which should be accessable from the client should be located in the folder `toradex/files`.

### Executing the update on the client

To run the update on the client, you may first have to create different versions of docker images (to see that the update actually works). You could do this by use the export command of docker and create a new image out of an existing one and apply slight changes for different versions. See [https://docs.docker.com/engine/reference/commandline/export/] for more details.  
_Be aware that the download size of the image is limited to 100MB. This is hard coded in the a71chtdx.c. If you want to download bigger files, you need to adjust the memory allocation of the read buffer._

With the update script, you are also able to remove an existing image simultaneously to the update. You will be asked for an image ID before the update starts. You can also leave this blank.

The update itself uses `docker import` to create a new image out of a tar. This means, you need to ensure that the tar-file is properly exported from a docker container and that it is possible to import it.
