package ij.plugin;
import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.io.*;
import java.net.URL;
import java.awt.image.*;

/** This plugin implements the Help/About ImageJ command by opening
	the about.jpg in ij.jar, scaling it 400% and adding some text. */
	public class AboutBox implements PlugIn {
		static final int SMALL_FONT=14, LARGE_FONT=26;

	public void run(String arg) {
		int lines = 8;
		String[] text = new String[lines];
		text[0] = "Squiz Image Editor";
		text[1] = "based on ImageJ "+ImageJ.VERSION;
		text[2] = "by Wayne Rasband from the";
		text[3] = "National Institutes of Health, USA";
		text[4] = "http://rsb.info.nih.gov/ij/";
		text[5] = "Java "+System.getProperty("java.version");
		text[6] = IJ.freeMemory();
		text[7] = "ImageJ is in the public domain";
		ImageProcessor ip = null;
		ImageJ ij = IJ.getInstance();
		URL url = ij .getClass() .getResource("/about.jpg");
		if (url!=null) {
			Image img = null;
			try {img = ij.createImage((ImageProducer)url.getContent());}
			catch(Exception e) {}
			if (img!=null) {
				ImagePlus imp = new ImagePlus("", img);
				ip = imp.getProcessor();
			}
		}
		if (ip==null) 
			ip =  new ColorProcessor(70,45);
		ip = ip.resize(ip.getWidth()*4, ip.getHeight()*4);
		ip.setFont(new Font("SansSerif", Font.PLAIN, LARGE_FONT));
		ip.setAntialiasedText(true);
		int[] widths = new int[lines];
		widths[0] = ip.getStringWidth(text[0]);
		ip.setFont(new Font("SansSerif", Font.PLAIN, SMALL_FONT));
		for (int i=1; i<lines-1; i++)
			widths[i] = ip.getStringWidth(text[i]);
		int max = 0;
		max = ip.getWidth();
		ip.setColor(new Color(255,255,140));
		ip.setFont(new Font("SansSerif", Font.PLAIN, LARGE_FONT));
		int y  = 34;
		ip.drawString(text[0], x(text[0],ip,max), y);
		ip.setFont(new Font("SansSerif", Font.PLAIN, SMALL_FONT));
		y += 26;
		ip.drawString(text[1], x(text[1],ip,max), y);
		y += 18;
		ip.drawString(text[2], x(text[2],ip,max), y);
		y += 18;
		ip.drawString(text[3], x(text[3],ip,max), y);
		y += 18;
		ip.drawString(text[4], x(text[4],ip,max), y);
		if (IJ.maxMemory()>0L) {
			y += 18;
			ip.drawString(text[5], x(text[5],ip,max), y);
		}
		y += 18;
		ip.drawString(text[6], x(text[6],ip,max), y);
		y += 18;
		ip.drawString(text[7], x(text[7],ip,max), y);
		new ImagePlus("About ImageJ", ip).show();
	}

	int x(String text, ImageProcessor ip, int max) {
		return ip.getWidth() - max + (max - ip.getStringWidth(text))/2 - 10;
	}

}
