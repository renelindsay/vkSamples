/assets/ReadMe.txt
------------------
Place your asset files in this 'assets' folder.
On Android, the contents of this folder will be included in your APK file, 
and can be read using "fopen". Internally, On Android, 'fopen' uses AssetManager 
to read from this '/assets/' folder in the APK, but will be in read-only mode.
