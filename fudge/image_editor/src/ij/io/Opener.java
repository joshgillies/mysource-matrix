package ij.io;
import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.net.URL;
import java.net.*;
import java.util.zip.*;
import java.util.Iterator;
import java.util.Locale;
import javax.swing.*;
import javax.swing.filechooser.*;
import ij.*;
import ij.gui.*;
import ij.process.*;
import ij.plugin.frame.*;
import ij.text.TextWindow;
import ij.util.Java2;
import java.util.LinkedList;
import java.util.ArrayList;

/** Opens tiff, dicom, fits, pgm, jpeg, bmp or
	gif images, and look-up tables, using a file open dialog or a path.
	Calls HandleExtraFileTypes plugin if the file type is unrecognised. */
public class Opener {

	private static final int UNKNOWN=0,TIFF=1,DICOM=2,FITS=3,PGM=4,JPEG=5,
		GIF=6,LUT=7,BMP=8,ZIP=9,JAVA_OR_TEXT=10,ROI=11,TEXT=12,PNG=13,TIFF_AND_DICOM=14,CUSTOM=15;
	private static final String[] types = {"unknown","tif","dcm","fits","pgm",
		"jpg","gif","bmp","png"};
	public static FileExtensionFilter[] FILE_FILTERS;
	public static FileExtensionFilter ALL_FILES_FILTER;

	static {
		ArrayList filters = new ArrayList();
		LinkedList extensions = new LinkedList();
		extensions.add("gif");
		filters.add(new FileExtensionFilter("Graphics Interchange Format", extensions));
		extensions.clear();
		extensions.add("jpg");
		extensions.add("jpeg");
		filters.add(new FileExtensionFilter("Joint Photographic Experts Group", extensions));
		extensions.clear();
		extensions.add("png");
		filters.add(new FileExtensionFilter("Portable Network Graphics", extensions));
		extensions.clear();
		extensions.add("bmp");
		filters.add(new FileExtensionFilter("Windows Bitmap", extensions));
		extensions.clear();
		extensions.add("tif");
		extensions.add("tiff");
		filters.add(new FileExtensionFilter("Tagged Image File Format", extensions));
		extensions.clear();
		extensions.add("dcm");
		extensions.add("dicom");
		filters.add(new FileExtensionFilter("Digital Image and Communications in Medicine", extensions));
		extensions.clear();
		extensions.add("fits");
		filters.add(new FileExtensionFilter("Flexible Image Transport System", extensions));
		extensions.clear();
		extensions.add("pgm");
		filters.add(new FileExtensionFilter("Portable Gray Map", extensions));
		LinkedList allExtensions = new LinkedList();
		for (Iterator itr=filters.iterator(); itr.hasNext(); ) {
			String[] e = ((FileExtensionFilter)(itr.next())).getExtensions();
			for (int i=0; i < e.length; i++) {
				allExtensions.add(e[i]);
			}
		}
		ALL_FILES_FILTER = new FileExtensionFilter("All Supported Types", allExtensions);
		ALL_FILES_FILTER.setShowExtensions(false);

		FILE_FILTERS = new FileExtensionFilter[filters.size()+1];
		FILE_FILTERS[0] = ALL_FILES_FILTER;
		for (int i=0; i < filters.size(); i++) {
			FILE_FILTERS[i+1] = (FileExtensionFilter)filters.get(i);
		}

	}

	public static File defaultDirectory = null;
	private static int fileType;


	public Opener() {
	}

	/**
	* Present a JFileChooser for the user to select a file to open,
	* then open the file and display it
	*
	* @return boolean	Whether a file was successfully selected and displayed
	*/
	public boolean open() {
		JFileChooser fc = new JFileChooser();
		if (defaultDirectory != null) {
			fc.setCurrentDirectory(defaultDirectory);
		}
		for (int i=0; i < FILE_FILTERS.length; i++) {
			fc.addChoosableFileFilter(FILE_FILTERS[i]);
		}
		fc.setFileFilter(ALL_FILES_FILTER);
		fc.setAcceptAllFileFilterUsed(false);
		int returnVal = fc.showOpenDialog(IJ.getInstance());
		defaultDirectory = fc.getCurrentDirectory();

		if (returnVal!=JFileChooser.APPROVE_OPTION) {
			return false;
		}
		String path = fc.getSelectedFile().toString();
		return open(path);
	}


	/**
	* Open and display the specified file
	*
	* @param string		path	The path of the file to open
	* @return boolean	Whether the file was successfully displayed
	*/
	public boolean open(String path) {
		IJ.showStatus("Opening: " + path);
		
		ImagePlus imp = openImage(path);

		if (imp != null) {
			imp.show();
			IJ.getInstance().setFilename(path);
			return true;
		} else {
			String msg = "File is not in a valid format, or it was not found.";
			if (path!=null && path.length()<=64)
				msg += " \n  \n   "+path;
			IJ.showMessage("Opener", msg);
			return false;
		}
	}


	/**
	* Get an ImagePlus object from the specified path
	* 
	* @param string		path	The path to look in, may be a url or file path
	*
	* @return ImagePlus	The imageplus object from the file
	*/
	public ImagePlus openImage(String path) {
		ImagePlus img = null;
		if (path==null || path.equals(""))
			img = null;
		else if (path.indexOf("://")>0)
			img = openURL(path);
		else {
			img = openImage(getDir(path), getName(path));
		}
		return img;
	}


	/**
	* Get an ImagePlus object from the specified file in the specified dir
	* 
	* @param string		directory	The directory the file is in
	* @param string		name		The name of the file to open
	*
	* @return ImagePlus	The imageplus object from the file
	*/
	public ImagePlus openImage(String directory, String name) {
		ImagePlus imp;
		if (directory.length()>0 && !directory.endsWith(Prefs.separator))
			directory += Prefs.separator;
		String path = directory+name;
		fileType = getFileType(path,name);
		if (IJ.debugMode)
			IJ.log("openImage: \""+types[fileType]+"\", "+path);
		switch (fileType) {
			case TIFF:
				imp = openTiff(directory, name);
				return imp;
			case DICOM:
				imp = (ImagePlus)IJ.runPlugIn("ij.plugin.DICOM", path);
				if (imp.getWidth()!=0) return imp; else return null;
			case TIFF_AND_DICOM:
				// "hybrid" files created by GE-Senographe 2000 D */
				imp = openTiff(directory,name);
				ImagePlus imp2 = (ImagePlus)IJ.runPlugIn("ij.plugin.DICOM", path);
				if (imp!=null)				
					imp.setProperty("Info",imp2.getProperty("Info"));
				return imp;
			case FITS:
				imp = (ImagePlus)IJ.runPlugIn("ij.plugin.FITS", path);
				if (imp.getWidth()!=0) return imp; else return null;
			case PGM:
				imp = (ImagePlus)IJ.runPlugIn("ij.plugin.PGM_Reader", path);
				if (imp.getWidth()!=0) return imp; else return null;
			case GIF: 
				IJ.getInstance().setCurrentType(".gif");
				// and continuing...
			case JPEG: case PNG:
				imp = openJpegOrGif(directory, name);
				if (imp!=null&&imp.getWidth()!=0) return imp; else return null;
			case BMP:
				imp = (ImagePlus)IJ.runPlugIn("ij.plugin.BMP_Reader", path);
				if (imp.getWidth()!=0) return imp; else return null;
			case ZIP:
				imp = (ImagePlus)IJ.runPlugIn("ij.plugin.Zip_Reader", path);
				if (imp.getWidth()!=0) return imp; else return null;
			case UNKNOWN: case TEXT:
				// Call HandleExtraFileTypes plugin to see if it can handle unknown format
				imp = (ImagePlus)IJ.runPlugIn("HandleExtraFileTypes", path);
				if (imp==null) return null;
				if (imp.getWidth()>0 && imp.getHeight()>0) {
					fileType = CUSTOM;
					return imp;
				} else {
					if (imp.getWidth()==-1)
						fileType = CUSTOM; // plugin opened image so don't display error
					return null;
				}
			default:
				return null;
		}
	}

	/**
	* Get an imageplus object from a file URL
	*
	* @param string	url
	*
	* @return ImagePlus	The imageplus object from the file
	*/
	public ImagePlus openURL(String url) {
	   	try {
			String name = "";
			int index = url.lastIndexOf('/');
			if (index==-1)
				index = url.lastIndexOf('\\');
			if (index>0)
				name = url.substring(index+1);
			else
				throw new MalformedURLException("Invalid URL: "+url);
			URL u = new URL(url);
			IJ.showStatus(""+url);
			ImagePlus imp = null;
		    if (url.endsWith(".tif") || url.endsWith(".TIF"))
				imp = openTiff(u.openStream(), name);
	 	    else if (url.endsWith(".zip"))
				imp = openZip(u);
	 	    else if (url.endsWith(".dcm")) {
				imp = (ImagePlus)IJ.runPlugIn("ij.plugin.DICOM", url);
				if (imp!=null && imp.getWidth()==0) imp = null;
			} else
				imp = openJpegOrGifUsingURL(name, u);
			IJ.showStatus("");
			return imp;
    	} catch (Exception e) {
    		String msg = e.getMessage();
    		if (msg==null || msg.equals(""))
    			msg = "" + e;	
			IJ.showMessage("Open URL",msg + "\n \n" + url);
			return null;
	   	} 
	}
	
	/** Opens the ZIP compressed TIFF at the specified URL. */
	ImagePlus openZip(URL url) throws IOException {
		IJ.showProgress(0.01);
		URLConnection uc = url.openConnection();
		int fileSize = uc.getContentLength(); // compressed size
		fileSize *=2; // estimate uncompressed size
      	InputStream in = uc.getInputStream();
		ZipInputStream zin = new ZipInputStream(in);
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		byte[] buf = new byte[4096];
		ZipEntry entry = zin.getNextEntry();
		if (entry==null)
			return null;
		String name = entry.getName();
		//double fileSize = entry.getSize(); //returns -1
		if (!name.endsWith(".tif"))
			throw new IOException("This ZIP archive does not appear to contain a TIFF file");
		int len;
		int byteCount = 0;
		int progress = 0;
		while (true) {
			len = zin.read(buf);
			if (len<0) break;
			out.write(buf, 0, len);
			byteCount += len;
			IJ.showProgress((double)(byteCount%fileSize)/fileSize);
		}
		zin.close();
		byte[] bytes = out.toByteArray();
		IJ.showProgress(1.0);
		return openTiff(new ByteArrayInputStream(bytes), name);
	}

	ImagePlus openJpegOrGifUsingURL(String title, URL url) {
		if (url==null)
			return null;
    	Image img = Toolkit.getDefaultToolkit().getImage(url);
		if (img!=null) {
			ImagePlus imp = new ImagePlus(title, img);
			return imp;
		} else
			return null;
	}

	ImagePlus openJpegOrGif(String dir, String name) {
	   	ImagePlus imp = null;
		Image img = Toolkit.getDefaultToolkit().getImage(dir+name);
		if (img!=null) {
 			try {
 				imp = new ImagePlus(name, img);
 			} catch (IllegalStateException e) {
				return null; // error loading image				
 			} 
		
	    	if (imp.getType()==ImagePlus.COLOR_RGB)
	    		convertGrayJpegTo8Bits(imp);
	    	FileInfo fi = new FileInfo();
	    	fi.fileFormat = fi.GIF_OR_JPG;
	    	fi.fileName = name;
	    	fi.directory = dir;
	    	imp.setFileInfo(fi);
			
	    }
	    return imp;

	}
	
	/** If this image is grayscale, convert it to 8-bits. */
	public static void convertGrayJpegTo8Bits(ImagePlus imp) {
		ImageProcessor ip = imp.getProcessor();
		int width = ip.getWidth();
		int height = ip.getHeight();
		int[] pixels = (int[])ip.getPixels();
		int c,r,g,b,offset;
		for (int y=0; y<(height-8); y++) {
			offset = y*width;
			for (int x=0; x<(width-8); x++) {
				c = pixels[offset+x];
				r = (c&0xff0000)>>16;
				g = (c&0xff00)>>8;
				b = c&0xff;
				if (!((r==g)&&(g==b))) {
					//IJ.write("count: "+count+" "+r+" "+g+" "+b);
					return;
				}
			}
			//count++;
		}
		IJ.showStatus("Converting to 8-bits");
		new ImageConverter(imp).convertToGray8();
	}

	/** Attempts to open the specified file as a tiff.
		Returns an ImagePlus object if successful. */
	public ImagePlus openTiff(String directory, String name) {
		TiffDecoder td = new TiffDecoder(directory, name);
		if (IJ.debugMode) td.enableDebugging();
		FileInfo[] info=null;
		try {info = td.getTiffInfo();}
		catch (IOException e) {
			String msg = e.getMessage();
			if (msg==null||msg.equals("")) msg = ""+e;
			IJ.showMessage("TiffDecoder", msg);
			return null;
		}
		if (info==null)
			return null;
		return openTiff2(info);
	}
	
	/** Attempts to open the specified inputStream as a
		TIFF, returning an ImagePlus object if successful. */
	public ImagePlus openTiff(InputStream in, String name) {
		FileInfo[] info = null;
		try {
			TiffDecoder td = new TiffDecoder(in, name);
			if (IJ.debugMode) td.enableDebugging();
			info = td.getTiffInfo();
		} catch (FileNotFoundException e) {
			IJ.showMessage("TiffDecoder", "File not found: "+e.getMessage());
			return null;
		} catch (Exception e) {
			IJ.showMessage("TiffDecoder", ""+e);
			return null;
		}
		return openTiff2(info);
	}

	public String getName(String path) {
		int i = path.lastIndexOf('/');
		if (i==-1)
			i = path.lastIndexOf('\\');
		if (i>0)
			return path.substring(i+1);
		else
			return path;
	}
	
	public String getDir(String path) {
		int i = path.lastIndexOf('/');
		if (i==-1)
			i = path.lastIndexOf('\\');
		if (i>0)
			return path.substring(0, i+1);
		else
			return "";
	}

	/**
	* Helper for opening tiffs
	*/
	ImagePlus openTiff2(FileInfo[] info) {
		if (info==null)
			return null;
		ImagePlus imp = null;
		if (IJ.debugMode) // dump tiff tags
			IJ.log(info[0].info);
		FileOpener fo = new FileOpener(info[0]);
		imp = fo.open(false);
		IJ.showStatus("");
		return imp;
	}
	

	/**
	Attempts to determine the image file type by looking for
	'magic numbers' or text strings in the header.
	 */
	int getFileType(String path, String name) {
		InputStream is;
		byte[] buf = new byte[132];
		try {
			is = new FileInputStream(path);
			is.read(buf, 0, 132);
			is.close();
		} catch (IOException e) {
			IJ.error("Couldn't open path "+path);
			return UNKNOWN;
		}
		
		int b0=buf[0]&255, b1=buf[1]&255, b2=buf[2]&255, b3=buf[3]&255;
		//IJ.log("getFileType: "+ name+" "+b0+" "+b1+" "+b2+" "+b3);
		
		 // Combined TIFF and DICOM created by GE Senographe scanners
		if (buf[128]==68 && buf[129]==73 && buf[130]==67 && buf[131]==77
		&& ((b0==73 && b1==73)||(b0==77 && b1==77)))
			return TIFF_AND_DICOM;

		 // Big-endian TIFF ("MM")
        if (name.endsWith(".lsm"))
        		return UNKNOWN; // The LSM  Reader plugin opens these files
		if (b0==73 && b1==73 && b2==42 && b3==0)
				return TIFF;

		 // Little-endian TIFF ("II")
		if (b0==77 && b1==77 && b2==0 && b3==42)
				return TIFF;

		 // JPEG
		if (b0==255 && b1==216 && b2==255)
			return JPEG;

		 // GIF ("GIF8")
		if (b0==71 && b1==73 && b2==70 && b3==56)
			return GIF;

		 // DICOM ("DICM" at offset 128)
		if (buf[128]==68 && buf[129]==73 && buf[130]==67 && buf[131]==77) {
			return DICOM;
		}

 		// ACR/NEMA with first tag = 00002,00xx or 00008,00xx
		name = name.toLowerCase(Locale.US);
 		if ((b0==8||b0==2) && b1==0 && b3==0 && !name.endsWith(".spe")) 	
  			 	return DICOM;

		// FITS ("SIMP")
		if (b0==83 && b1==73 && b2==77 && b3==80)
			return FITS;
			
		// PGM ("P2" or "P5")
		if (b0==80&&(b1==50||b1==53)&&(b2==10||b2==13||b2==32||b2==9))
			return PGM;

		// Lookup table
		if (name.endsWith(".lut"))
			return LUT;
		
		// BMP ("BM")
		if (b0==66 && b1==77 && name.endsWith(".bmp"))
			return BMP;
				
		// PNG
		if (b0==137 && b1==80 && b2==78 && b3==71 && IJ.isJava2())
			return PNG;
				
		// ZIP containing a TIFF
		if (name.endsWith(".zip"))
			return ZIP;

		// Java source file or text file
		if (name.endsWith(".java") || name.endsWith(".txt"))
			return JAVA_OR_TEXT;

		// ImageJ, NIH Image, Scion Image for Windows ROI
		if (b0==73 && b1==111) // "Iout"
			return ROI;
			
        // Text file
        boolean isText = true;
        for (int i=0; i<10; i++) {
          int c = buf[i];
          if ((c<32&&c!=9&&c!=10&&c!=13) || c>126) {
              isText = false;
              break;
          }
        }
        if (isText)
           return TEXT;

		return UNKNOWN;
	}

	/** Returns an IndexColorModel for the image specified by this FileInfo. */
	ColorModel createColorModel(FileInfo fi) {
		if (fi.fileType==FileInfo.COLOR8 && fi.lutSize>0)
			return new IndexColorModel(8, fi.lutSize, fi.reds, fi.greens, fi.blues);
		else
			return LookUpTable.createGrayscaleColorModel(fi.whiteIsZero);
	}

	/** Returns an InputStream for the image described by this FileInfo. */
	InputStream createInputStream(FileInfo fi) throws IOException, MalformedURLException {
		if (fi.inputStream!=null)
			return fi.inputStream;
		else if (fi.url!=null && !fi.url.equals(""))
			return new URL(fi.url+fi.fileName).openStream();
		else {
		    File f = new File(fi.directory + fi.fileName);
		    if (f==null || f.isDirectory())
		    	return null;
		    else
				return new FileInputStream(f);
		}
	}

}
