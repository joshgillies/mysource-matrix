package ij.process;

import java.awt.*;
import java.awt.image.*;
import ij.*;
import ij.gui.*;
import ij.measure.*;

/** This class converts an ImagePlus object to a different type. */
public class ImageConverter {
	private ImagePlus imp;
	private int type;
	//private static boolean doScaling = Prefs.getBoolean(Prefs.SCALE_CONVERSIONS,true);
	private static boolean doScaling = true;

	/** Constructs an ImageConverter based on an ImagePlus object. */
	public ImageConverter(ImagePlus imp) {
		this.imp = imp;
		type = imp.getType();
	}

	/** Converts this ImagePlus to 8-bit grayscale. */
	public synchronized void convertToGray8() {
		ImageProcessor ip = imp.getProcessor();
		if (type==ImagePlus.GRAY16 || type==ImagePlus.GRAY32) {
			imp.setProcessor(null, ip.convertToByte(doScaling));
			imp.setCalibration(imp.getCalibration()); //update calibration
		} else if (type==ImagePlus.COLOR_RGB)
	    	imp.setProcessor(null, ip.convertToByte(doScaling));
		else if (ip.isPseudoColorLut()) {
			boolean invertedLut = ip.isInvertedLut();
			ip.setColorModel(LookUpTable.createGrayscaleColorModel(invertedLut));
	    	imp.updateAndDraw();
		} else {
			ip = new ColorProcessor(imp.getImage());
	    	imp.setProcessor(null, ip.convertToByte(doScaling));
	    }
	}

	/** Converts this ImagePlus to 16-bit grayscale. */
	public void convertToGray16() {
		if (type==ImagePlus.GRAY16)
			return;
		if (!(type==ImagePlus.GRAY8 || type==ImagePlus.GRAY32))
			throw new IllegalArgumentException("Unsupported conversion");
		ImageProcessor ip = imp.getProcessor();
		imp.trimProcessor();
		imp.setProcessor(null, ip.convertToShort(doScaling));
		imp.setCalibration(imp.getCalibration()); //update calibration
	}

	/** Converts this ImagePlus to 32-bit grayscale. */
	public void convertToGray32() {
		if (type==ImagePlus.GRAY32)
			return;
		if (!(type==ImagePlus.GRAY8 || type==ImagePlus.GRAY16))
			throw new IllegalArgumentException("Unsupported conversion");
		ImageProcessor ip = imp.getProcessor();
		imp.trimProcessor();
		Calibration cal = imp.getCalibration();
		ip.setCalibrationTable(cal.getCTable());
		imp.setProcessor(null, ip.convertToFloat());
		imp.setCalibration(cal); //update calibration
	}

	/** Converts this ImagePlus to RGB. */
	public void convertToRGB() {
		ImageProcessor ip = imp.getProcessor();
		imp.setProcessor(null, ip.convertToRGB());
		imp.setCalibration(imp.getCalibration()); //update calibration
	}
	
	
	/** Converts an RGB image to 8-bits indexed color. 'nColors' must
		be greater than 1 and less than or equal to 256. */
	public void convertRGBtoIndexedColor(int nColors) {
		if (type!=ImagePlus.COLOR_RGB)
			throw new IllegalArgumentException("Image must be RGB");
		if (nColors<2) nColors = 2;
		if (nColors>256) nColors = 256;
		
		// get RGB pixels
		IJ.showProgress(0.1);
		IJ.showStatus("Grabbing pixels");
		int width = imp.getWidth();
		int height = imp.getHeight();
		ImageProcessor ip = imp.getProcessor();
	 	ip.snapshot();
		int[] pixels = (int[])ip.getPixels();
		imp.trimProcessor();
		
		// convert to 8-bits
		long start = System.currentTimeMillis();
		MedianCut mc = new MedianCut(pixels, width, height);
		ImageProcessor ip2 = mc.convertToByte(nColors);
	    imp.setProcessor(null, ip2);
	}
	
	/** Set true to scale to 0-255 when converting short to byte or float
		to byte and to 0-65535 when converting float to short. */
	public static void setDoScaling(boolean scaleConversions) {
		doScaling = scaleConversions;
	}

	/** Returns true if scaling is enabled. */
	public static boolean getDoScaling() {
		return doScaling;
	}
}
