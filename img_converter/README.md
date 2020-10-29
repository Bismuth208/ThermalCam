# Iamge converter for ThermalCam
***

#### Things to do before use

Step 1:
install pip for python via any possible way what you like or find
[or follow this](https://pip.pypa.io/en/stable/installing/ "Installing with get-pip.py");

Step 2:
type in terminal\cmd line:
```
pip install image
```

or in some cases:
```
pip install PIL
```

#### RAW thermograms to PNG 

Copy folder "mlx" from root of SD card to your Desktop machine.
Open terminal\cmd line and navigate inside "mlx" folder which already was copied.

Type:
```
python thermal_conv.py -d ./mlx/XXXX/
```
where XXXX name of folder whith .thc files inside.

After execution in folder MUST appear .png files.


#### PNG thermograms to GIF

While you still in same folder type:
```
python thermal_gif.py -d ./mlx/XXXX/
```
where XXXX name of folder whith .png files inside.

After execution in folder MUST apper .gif file.

