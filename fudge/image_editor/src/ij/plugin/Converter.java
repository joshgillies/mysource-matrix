package ij.plugin;
import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;

/** Implements the conversion commands in the Image/Type submenu. */
public class Converter implements PlugIn {

	private ImagePlus imp;

	public void run(String arg) {
		imp = IJ.getInstance().getImagePlus();
		if (imp!=null) {
			if (imp.lock()) { //v1.24f
				convert(arg);
				imp.unlock();
			}
		} else
			IJ.noImage();
	}

	/** Converts the ImagePlus to the specified image type. The string
	argument corresponds to one of the labels in the Image/Type submenu
	("8-bit", "16-bit", "32-bit", "8-bit Color", "RGB Color", "RGB Stack" or "HSB Stack"). */
	public void convert(String item) {
		int type = imp.getType();
		String msg = "Converting to " + item;
		IJ.showStatus(msg + "...");
	 	long start = System.currentTimeMillis();
	 	boolean isRoi = imp.getRoi()!=null;
	 	imp.killRoi();
	 	boolean saveChanges = imp.changes;
		imp.changes = IJ.getApplet()==null; //if not applet, set 'changes' flag
		try {
			// do single image conversions
			Undo.setup(Undo.TYPE_CONVERSION, imp);
			ImageConverter ic = new ImageConverter(imp);
			if (item.equals("8-bit"))
				ic.convertToGray8();
			else if (item.equals("16-bit"))
				ic.convertToGray16();
			else if (item.equals("32-bit"))
				ic.convertToGray32();
			else if (item.equals("RGB Color")) {
				ic.convertToRGB();
			} else if (item.equals("8-bit Color")) {
				int nColors = getNumber();
				start = System.currentTimeMillis();
				if (nColors!=0)
					ic.convertRGBtoIndexedColor(nColors);
			} else {
				imp.changes = saveChanges;
				return;
			}
			IJ.showProgress(1.0);
			
		}
		catch (IllegalArgumentException e) {
			unsupportedConversion(imp);
			IJ.showStatus("");
	    	Undo.reset();
	    	imp.changes = saveChanges;
			Menus.updateMenus();
			return;
		}
		if (isRoi)
			imp.restoreRoi();
		IJ.showTime(imp, start, "");
		imp.repaintWindow();
		Menus.updateMenus();
	}

	void unsupportedConversion(ImagePlus imp) {
		IJ.showMessage("Converter",
			"Supported Conversions:\n" +
			" \n" +
			"8-bit -> 16-bit*\n" +
			"8-bit -> 32-bit*\n" +
			"8-bit -> RGB Color*\n" +
			"16-bit -> 8-bit*\n" +
			"16-bit -> 32-bit*\n" +
			"16-bit -> RGB Color*\n" +
			"32-bit -> 8-bit*\n" +
			"32-bit -> 16-bit\n" +
			"32-bit -> RGB Color*\n" +
			"8-bit Color -> 8-bit (grayscale)*\n" +
			"8-bit Color -> RGB Color\n" +
			"RGB Color -> 8-bit (grayscale)*\n" +
			"RGB Color -> 8-bit Color*\n" +
			"RGB Color -> RGB Stack\n" +
			"RGB Color -> HSB Stack\n" +
			"RGB Stack -> RGB Color\n" +
			"HSB Stack -> RGB Color\n" +
			" \n" +
			"* works with stacks\n"
			);
	}

	int getNumber() {
		if (imp.getType()!=ImagePlus.COLOR_RGB)
			return 256;
		GenericDialog gd = new GenericDialog("MedianCut");
		gd.addNumericField("Number of Colors (2-256):", 256, 0);
		gd.showDialog();
		if (gd.wasCanceled())
			return 0;
		int n = (int)gd.getNextNumber();
		if (n<2) n = 2;
		if (n>256) n = 256;
		return n;
	}
		
}
