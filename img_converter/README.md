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

#### RAW thermograms to PNG and/or GIF

Copy folder "mlx" from root of SD card to your Desktop machine.
Open terminal\cmd line and navigate inside "mlx" folder which already was copied.

Type:
```
python3 thermal_conv.py -d ./mlx/XXXX/
```
where XXXX name of folder with .thc files inside.

After execution in folder MUST appear .png and/or .gif files.
