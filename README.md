# A71CH Secure Module Demo

This demonstration is used for the Security Workshop on the 21th May 2019 in Munich.

- Use of the NxP A71CH Secure Module
- Combination with the Colibri iMX6ULL
- Combination with Torizon

Still under development.

## Preperation

### Building of the libraries and the A71CH tools

### Creating the certificates

First of all, the certificates need to be created. NxP provides a script in their host library to the A71CH Secure Element module. You can either edit the script and change the properties of the X.509 certificate which should be generated or take the default values and run it directly. The script is located under `hostLib/embSeEngine/a71chDemo/scripts/tlsCreateCredentialsRunOnClientOnce.sh`

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

For the provisioning of the A71CH Secure Element, the configuration application (`a71chConfig_i2c_imx`) which we compiled in chapter XX is required. Copy this tool to the target module.

NxP provides a script to do the provisioning of the A71CH Secure Element. Copy the script located under `hostLib/embSeEngine/a71chDemo/scripts/tlsPrepareClient.sh` to the target module. Check the references to the cert-files in the shell-script and change them to whereever you stored your cert-files. The path of `client_key_ref` points to the location where the reference key should be stored in future and the file itself does not exist yet (will be created with the provisioning). Please also check the location of the `probeExec` which should point to the copied `a71chConfig_i2c_imx`.

For the provisioning of the A71CH Secure Element, you should connect the A71CH properly and ensure that the module is in Debug Mode. If the module is not in debug mode, you could use the config-utility (Chapter XX) to configure it. If everything is okay, run the script.

### Building the a71tdx application (Demo application)

## Running the demo