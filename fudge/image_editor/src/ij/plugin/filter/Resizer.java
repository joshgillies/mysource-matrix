package ij.plugin.filter;
import ij.*;
import ij.gui.*;
import ij.process.*;
import ij.measure.*;
import java.awt.*;

/** This plugin implements ImageJ's Resize command. */
public class Resizer implements PlugInFilter {
	ImagePlus imp;
	private boolean crop;
    private static int newWidth = 100;
    private static int newHeight = 100;
    private static boolean constrain = true;
    private static boolean interpolate = true;

	public int setup(String arg, ImagePlus imp) {
		crop = arg.equals("crop");
		this.imp = imp;
		IJ.register(Resizer.class);
		if (crop)
			return DOES_ALL+ROI_REQUIRED+NO_CHANGES;
		else
			return DOES_ALL+NO_CHANGES;
	}

	public void run(ImageProcessor ip) {
		Roi roi = imp.getRoi();
		if (roi!=null && roi.getType()>=Roi.LINE && roi.getType()<=Roi.FREELINE) {
			IJ.error("The Crop and Adjust->Size commands\ndo not work with line selections.");
			return;
		}
		boolean sizeToHeight=false;
		if (crop) {
			Rectangle bounds = roi.getBounds();
			newWidth = bounds.width;
			newHeight = bounds.height;
			interpolate = false;
		} else {
			GenericDialog gd = new GenericDialog("Resize", IJ.getInstance());
			gd.addNumericField("Width (pixels):", newWidth, 0);
			gd.addNumericField("Height (pixels):", newHeight, 0);
			gd.addCheckbox("Constrain Aspect Ratio", constrain);
			gd.addCheckbox("Interpolate", interpolate);
			gd.addMessage("NOTE: Undo is not available");
			gd.showDialog();
			if (gd.wasCanceled())
				return;
			newWidth = (int)gd.getNextNumber();
			newHeight = (int)gd.getNextNumber();
			if (gd.invalidNumber()) {
				IJ.error("Width or height are invalid.");
				return;
			}
			constrain = gd.getNextBoolean();
			interpolate = gd.getNextBoolean();
			sizeToHeight = constrain && newWidth==0;
			if (newWidth<=0.0 && !constrain)  newWidth = 50;
			if (newHeight<=0.0) newHeight = 50;
		}
		
		Rectangle r = ip.getRoi();
		double oldWidth = r.width;;
		double oldHeight = r.height;
		if (!crop && constrain) {
			if (sizeToHeight)
				newWidth = (int)(newHeight*(oldWidth/oldHeight));
			else
				newHeight = (int)(newWidth*(oldHeight/oldWidth));
		}
		ip.setInterpolate(interpolate);
    	
		try {
			ip = ip.resize(newWidth, newHeight);
			ImagePlus imp2 = new ImagePlus(imp.getTitle(), ip);
	    	imp2.show();

		} catch(OutOfMemoryError o) {
			IJ.outOfMemory("Resize");
		}
	}

}