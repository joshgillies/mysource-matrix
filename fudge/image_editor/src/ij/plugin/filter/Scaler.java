package ij.plugin.filter;
import ij.*;
import ij.gui.*;
import ij.process.*;
import ij.measure.*;
import java.awt.*;

/** This plugin implements the Edit/Scale command. */
public class Scaler implements PlugInFilter {
    private ImagePlus imp;
    private static double xscale = 0.5;
    private static double yscale = 0.5;
    private static boolean interpolate = true;

	public int setup(String arg, ImagePlus imp) {
		this.imp = imp;
		IJ.register(Scaler.class);
		if (imp!=null) {
			Roi roi = imp.getRoi();
			if (roi!=null && !roi.isArea())
				imp.killRoi(); // ignore any line selection
		}
		return DOES_ALL;
	}

	public void run(ImageProcessor ip) {
		if (!showDialog())
			return;
		ip.setInterpolate(interpolate);
		imp.startTiming();
		try {
			scale(ip);
		}
		catch(OutOfMemoryError o) {
			IJ.outOfMemory("Scale");
		}
		IJ.showProgress(1.0);
	}
	
	void scale(ImageProcessor ip) {
		ip.scale(xscale, yscale);
		imp.killRoi();
	}
	
	boolean showDialog() {
		GenericDialog gd = new GenericDialog("Scale");
		gd.addNumericField("X Scale (0.05-25):", xscale, 2);
		gd.addNumericField("Y Scale (0.05-25):", yscale, 2);
		gd.addCheckbox("Interpolate", interpolate);
		gd.showDialog();

		if (gd.wasCanceled())
			return false;
		xscale = gd.getNextNumber();
		yscale = gd.getNextNumber();
		if (gd.invalidNumber()) {
			IJ.error("X or Y scale are invalid.");
			return false;
		}
		if (xscale > 25.0) xscale = 25.0;
		if (xscale < 0.05) xscale = 0.05;
		if (yscale > 25.0) yscale = 25.0;
		if (yscale < 0.05) yscale = 0.05;
		interpolate = gd.getNextBoolean();
		return true;
	}

}