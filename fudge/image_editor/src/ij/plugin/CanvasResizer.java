package ij.plugin;
import ij.*;
import ij.plugin.filter.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;

/** This plugin implements the Image/Adjust/Canvas Size command.
	It changes the canvas size of an image without resizing the image.
	The border is filled with the current background color.
	@author Jeffrey Kuhn (jkuhn at ccwf.cc.utexas.edu)
*/
public class CanvasResizer implements PlugIn {
	boolean zeroFill = Prefs.get("resizer.zero", false);

	public void run(String arg) {
		int wOld, hOld, wNew, hNew;

		ImagePlus imp = IJ.getImage();
		wOld = imp.getWidth();
		hOld = imp.getHeight();

		String[] sPositions = {
			"Top-Left", "Top-Center", "Top-Right", 
			"Center-Left", "Center", "Center-Right",
			"Bottom-Left", "Bottom-Center", "Bottom-Right"
		};
			
		String strTitle = "Resize Image Canvas";
		GenericDialog gd = new GenericDialog(strTitle);
		gd.addNumericField("Width:", wOld, 0, 5, "pixels");
		gd.addNumericField("Height:", hOld, 0, 5, "pixels");
		gd.addChoice("Position:", sPositions, sPositions[4]);
		gd.addCheckbox("Zero Fill", zeroFill);
		gd.showDialog();
		if (gd.wasCanceled())
			return;
			
		wNew = (int)gd.getNextNumber();
		hNew = (int)gd.getNextNumber();
		int iPos = gd.getNextChoiceIndex();
		zeroFill = gd.getNextBoolean();
		Prefs.set("resizer.zero", zeroFill);
		
		int xOff, yOff;
		int xC = (wNew - wOld)/2;	// offset for centered
		int xR = (wNew - wOld);		// offset for right
		int yC = (hNew - hOld)/2;	// offset for centered
		int yB = (hNew - hOld);		// offset for bottom
		
		switch(iPos) {
		case 0:	// TL
			xOff=0;	yOff=0; break;
		case 1:	// TC
			xOff=xC; yOff=0; break;
		case 2:	// TR
			xOff=xR; yOff=0; break;
		case 3: // CL
			xOff=0; yOff=yC; break;
		case 4: // C
			xOff=xC; yOff=yC; break;
		case 5:	// CR
			xOff=xR; yOff=yC; break;
		case 6: // BL
			xOff=0; yOff=yB; break;
		case 7: // BC
			xOff=xC; yOff=yB; break;
		case 8: // BR
			xOff=xR; yOff=yB; break;
		default: // center
			xOff=xC; yOff=yC; break;
		}
		
		Undo.setup(Undo.COMPOUND_FILTER, imp);
		ImageProcessor newIP = expandImage(imp.getProcessor(), wNew, hNew, xOff, yOff);
		imp.setProcessor(null, newIP);
		Undo.setup(Undo.COMPOUND_FILTER_DONE, imp);
	}
	
	public ImageProcessor expandImage(ImageProcessor ipOld, int wNew, int hNew, int xOff, int yOff) {
		ImageProcessor ipNew = ipOld.createProcessor(wNew, hNew);
		if (zeroFill)
			ipNew.setValue(0.0);
		else 
			ipNew.setColor(Toolbar.getBackgroundColor());
		ipNew.fill();
		ipNew.insert(ipOld, xOff, yOff);
		return ipNew;
	}

}

