package ij.plugin.filter;
import ij.*;
import ij.process.*;
import ij.gui.*;
import ij.measure.Calibration;
import java.awt.*;
import java.awt.image.*;

/** Implements the flip and rotate commands in the Image/Transformations submenu. */
public class Transformer implements PlugInFilter {
	
	ImagePlus imp;
	String arg;

	public int setup(String arg, ImagePlus imp) {
		this.arg = arg;
		this.imp = imp;
		if (arg.equals("fliph") || arg.equals("flipv"))
			return IJ.setupDialog(imp, DOES_ALL+NO_UNDO);
		else
			return DOES_ALL+NO_UNDO+NO_CHANGES;
	}

	public void run(ImageProcessor ip) {

		if (arg.equals("fliph")) {
			ip.flipHorizontal();
			return;
		}
		
		if (arg.equals("flipv")) {
			ip.flipVertical();
			return;
		}
		
		if (arg.equals("right"))
    		ip = ip.rotateRight();
    	else
    		ip = ip.rotateLeft();
    	Calibration cal1 = imp.getCalibration();
    	imp.changes = false;
    	ImagePlus imp2 = new ImagePlus(imp.getTitle(), ip);
    	imp2.show();
		return;
	}

}
