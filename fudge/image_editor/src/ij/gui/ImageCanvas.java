package ij.gui;

import java.awt.*;
import java.util.Properties;
import java.awt.image.*;
import java.awt.event.*;
import ij.process.ImageProcessor;
import ij.measure.*;
import ij.plugin.frame.Recorder;
import ij.*;
import ij.util.Java2;
import javax.swing.*;

/** This is a Canvas used to display images in a Window. */
public class ImageCanvas extends JPanel implements MouseListener, MouseMotionListener, Cloneable {

	protected static Cursor defaultCursor = new Cursor(Cursor.DEFAULT_CURSOR);
	protected static Cursor handCursor = new Cursor(Cursor.HAND_CURSOR);
	protected static Cursor moveCursor = new Cursor(Cursor.MOVE_CURSOR);
	protected static Cursor crosshairCursor = new Cursor(Cursor.CROSSHAIR_CURSOR);

	protected static ImagePlus clipboard;


	public static boolean usePointer = Prefs.usePointerCursor;

	static final int NO_MODS=0, ADD_TO_ROI=1, SUBTRACT_FROM_ROI=2; // ROI modification states

	protected ImagePlus imp;
	protected boolean imageUpdated;
	protected Rectangle srcRect;
	protected int imageWidth, imageHeight;
	protected int xMouse; // current cursor offscreen x location
	protected int yMouse; // current cursor offscreen y location

	private ImageJ ij;
	private double magnification;

	private int xMouseStart;
	private int yMouseStart;
	private int xSrcStart;
	private int ySrcStart;
	private int flags;

	int roiModState = NO_MODS;

	public ImageCanvas(ImagePlus imp) {
		this.imp = imp;
		ij = IJ.getInstance();
		int width = imp.getWidth();
		int height = imp.getHeight();
		imageWidth = width;
		imageHeight = height;
		srcRect = new Rectangle(0, 0, imageWidth, imageHeight);
		addMouseListener(this);
		addMouseMotionListener(this);
		addKeyListener(ij);  // ImageJ handles keyboard shortcuts
		Dimension viewportSize = ij.getViewportSize();
		//magnification = 1.0;
		magnification = Math.min(1.0, Math.min((float)viewportSize.width / (float)imageWidth, (float)(viewportSize.height - 20) / (float)imageHeight));

		if (Prefs.blackCanvas && getClass().getName().equals("ij.gui.ImagePanel")) {
			setForeground(Color.white);
			setBackground(Color.black);
		} else {
			setForeground(Color.black);
			setBackground(Color.white);
		}

		addKeyListener(ij);
	}

	public ImagePlus getImagePlus()
	{
		return this.imp;
	}


	/** ImagePlus.updateAndDraw calls this method to get paint
		to update the image from the ImageProcessor. */
	public void setImageUpdated() {
		imageUpdated = true;
	}

	public void update(Graphics g) {
		paint(g);
	}

	public void paintComponent(Graphics g) {
		Roi roi = imp.getRoi();
		if (roi != null) roi.updatePaste();
		try {
			if (imageUpdated) {
				imageUpdated = false;
				imp.updateImage();
			}
			if (IJ.isJava2()) {
				if (magnification<1.0)
					 Java2.setBilinearInterpolation(g, true);
				else if (IJ.isMacOSX())
					Java2.setBilinearInterpolation(g, false);
			}
			Image img = imp.getImage();
			if (img!=null)
				g.drawImage(img, 0, 0, (int)(srcRect.width*magnification), (int)(srcRect.height*magnification),
				srcRect.x, srcRect.y, srcRect.x+srcRect.width, srcRect.y+srcRect.height, null);
			if (roi != null)
				roi.draw(g);
		}
		catch(OutOfMemoryError e) {IJ.outOfMemory("Paint");}
	}


	/** Returns the current cursor location. */
	public Point getCursorLoc() {
		return new Point(xMouse, yMouse);
	}

	/** Returns the mouse event modifiers. */
	public int getModifiers() {
		return flags;
	}

	/** Sets the cursor based on the current tool and cursor location. */
	public void setCursor(int sx, int sy, int ox, int oy) {
		xMouse = ox;
		yMouse = oy;
		Roi roi = imp.getRoi();
		if (IJ.spaceBarDown()) {
			setCursor(handCursor);
			return;
		}
		switch (Toolbar.getToolId()) {
			case Toolbar.MAGNIFIER:
				if (IJ.isMacintosh())
					setCursor(defaultCursor);
				else
					setCursor(moveCursor);
				break;
			case Toolbar.HAND:
				setCursor(handCursor);
				break;
			default:  //selection tool
				if (roi!=null && roi.getState()!=roi.CONSTRUCTING && roi.isHandle(sx, sy)>=0)
					setCursor(handCursor);
				else if (usePointer || (roi!=null && roi.getState()!=roi.CONSTRUCTING && roi.contains(ox, oy)))
					setCursor(defaultCursor);
				else
					setCursor(crosshairCursor);
		}
	}

	/**Converts a screen x-coordinate to an offscreen x-coordinate.*/
	public int offScreenX(int x) {
		return srcRect.x + (int)(x/magnification);
	}

	/**Converts a screen y-coordinate to an offscreen y-coordinate.*/
	public int offScreenY(int y) {
		return srcRect.y + (int)(y/magnification);
	}

	/**Converts an offscreen x-coordinate to a screen x-coordinate.*/
	public int screenX(int x) {
		return  (int)((x-srcRect.x)*magnification);
	}

	/**Converts an offscreen y-coordinate to a screen y-coordinate.*/
	public int screenY(int y) {
		return  (int)((y-srcRect.y)*magnification);
	}

	public double getMagnification() {
		return magnification;
	}

	public void setMagnification(double magnification) {
		this.magnification = magnification;
		imp.setTitle(imp.getTitle());
		setSize((int)(imp.getWidth() * magnification), (int)(imp.getHeight() * magnification));
		setPreferredSize(getSize());
		revalidate();
		ij.repaint();
	}

	public Rectangle getSrcRect() {
		return srcRect;
	}

	private static final double[] zoomLevels = {
		1/72.0, 1/48.0, 1/32.0, 1/24.0, 1/16.0, 1/12.0,
		1/8.0, 1/6.0, 1/4.0, 1/3.0, 1/2.0, 0.75, 1.0,
		2.0, 3.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0, 32.0 };

	static double getLowerZoomLevel(double currentMag) {
		double newMag = zoomLevels[0];
		for (int i=0; i<zoomLevels.length; i++) {
			if (zoomLevels[i] < currentMag)
				newMag = zoomLevels[i];
			else
				break;
		}
		return newMag;
	}

	static double getHigherZoomLevel(double currentMag) {
		double newMag = 32.0;
		for (int i=zoomLevels.length-1; i>=0; i--) {
			if (zoomLevels[i]>currentMag)
				newMag = zoomLevels[i];
			else
				break;
		}
		return newMag;
	}

	/** Zooms in by making the window bigger. If we can't
		make it bigger, then make the srcRect smaller.*/
	public void zoomIn(int x, int y) {
		if (magnification>=32)
			return;
		double newMag = getHigherZoomLevel(magnification);
		setMagnification(newMag);
		repaint();
	}

	boolean canEnlarge(int newWidth, int newHeight) {
		return true;
	}

	/**Zooms out by making srcRect bigger. If we can't make
	it bigger, then make the window smaller.*/
	public void zoomOut(int x, int y) {
		if (magnification<=0.03125)
			return;
		double newMag = getLowerZoomLevel(magnification);
		setMagnification(newMag);
		ij.getContentPane().repaint();
	}

	void unzoom() {
		Dimension viewportSize = ij.getViewportSize();
		double imag = Math.min(1.0, Math.min((float)viewportSize.width / (float)imageWidth, (float)(viewportSize.height - 20) / (float)imageHeight));
		if (magnification==imag)
			return;
		setMagnification(imag);
		ij.getContentPane().repaint();
	}


	Color getColor(int index){
		IndexColorModel cm = (IndexColorModel)imp.getProcessor().getColorModel();
		//IJ.write(""+index+" "+(new Color(cm.getRGB(index))));
		return new Color(cm.getRGB(index));
	}

	protected void setDrawingColor(int ox, int oy, boolean setBackground) {
		//IJ.write("setDrawingColor: "+setBackground+this);
		int type = imp.getType();
		int[] v = imp.getPixel(ox, oy);
		switch (type) {
			case ImagePlus.GRAY8: {
				if (setBackground)
					setBackgroundColor(getColor(v[0]));
				else
					setForegroundColor(getColor(v[0]));
				break;
			}
			case ImagePlus.GRAY16: case ImagePlus.GRAY32: {
				double min = imp.getProcessor().getMin();
				double max = imp.getProcessor().getMax();
				double value = (type==ImagePlus.GRAY32)?Float.intBitsToFloat(v[0]):v[0];
				int index = (int)(255.0*((value-min)/(max-min)));
				if (index<0) index = 0;
				if (index>255) index = 255;
				if (setBackground)
					setBackgroundColor(getColor(index));
				else
					setForegroundColor(getColor(index));
				break;
			}
			case ImagePlus.COLOR_RGB: case ImagePlus.COLOR_256: {
				Color c = new Color(v[0], v[1], v[2]);
				if (setBackground)
					setBackgroundColor(c);
				else
					setForegroundColor(c);
				break;
			}
		}
		Color c;
		if (setBackground)
			c = Toolbar.getBackgroundColor();
		else {
			c = Toolbar.getForegroundColor();
			imp.setColor(c);
		}
		IJ.showStatus("("+c.getRed()+", "+c.getGreen()+", "+c.getBlue()+")");
	}

	private void setForegroundColor(Color c) {
		Toolbar.setForegroundColor(c);
		if (Recorder.record)
			Recorder.record("setForegroundColor", c.getRed(), c.getGreen(), c.getBlue());
	}

	private void setBackgroundColor(Color c) {
		Toolbar.setBackgroundColor(c);
		if (Recorder.record)
			Recorder.record("setBackgroundColor", c.getRed(), c.getGreen(), c.getBlue());
	}

	public void mousePressed(MouseEvent e) {
		if (ij==null) return;
		int toolID = Toolbar.getToolId();

		int x = e.getX();
		int y = e.getY();

		flags = e.getModifiers();
		if (IJ.debugMode) IJ.log("Mouse pressed: (" + x + "," + y + ")" + ij.modifiers(flags));
		//if (toolID!=Toolbar.MAGNIFIER && e.isPopupTrigger()) {
		if (toolID!=Toolbar.MAGNIFIER && (e.isPopupTrigger() || (flags & Event.META_MASK)!=0)) {
			handlePopupMenu(e);
			return;
		}

		int ox = offScreenX(x);
		int oy = offScreenY(y);
		xMouse = ox; yMouse = oy;
		switch (toolID) {
			case Toolbar.MAGNIFIER:
				if ((flags & (Event.ALT_MASK|Event.META_MASK|Event.CTRL_MASK))!=0)
					zoomOut(x, y);
				else
					zoomIn(x, y);
				break;
			case Toolbar.DROPPER:
				setDrawingColor(ox, oy, IJ.altKeyDown());
				break;
			case Toolbar.CROSSHAIR:
				IJ.doCommand("Measure");
				break;
			case Toolbar.WAND:
				Roi roi = imp.getRoi();
				if (roi!=null && roi.contains(ox, oy)) {
					Rectangle r = roi.getBounds();
					if (r.width==imageWidth && r.height==imageHeight)
						imp.killRoi();
					else {
						handleRoiMouseDown(e);
						return;
					}
				}
				if (roi!=null) {
					int handle = roi.isHandle(x, y);
					if (handle>=0) {
						roi.mouseDownInHandle(handle, x, y);
						return;
					}
				}
				setRoiModState(e, roi, -1);
				int npoints = IJ.doWand(ox, oy);
				if (Recorder.record && npoints>0)
					Recorder.record("doWand", ox, oy);
				break;
			default:  //selection tool
				handleRoiMouseDown(e);
		}
	}

	protected void handlePopupMenu(MouseEvent e) {
		if (IJ.debugMode) IJ.log("show popup: " + (e.isPopupTrigger()?"true":"false"));
		int x = e.getX();
		int y = e.getY();
		Roi roi = imp.getRoi();
		if (roi!=null && (roi.getType()==Roi.POLYGON || roi.getType()==Roi.POLYLINE || roi.getType()==Roi.ANGLE)
		&& roi.getState()==roi.CONSTRUCTING) {
			roi.handleMouseUp(x, y); // simulate double-click to finalize
			roi.handleMouseUp(x, y); // polygon or polyline selection
			return;
		}
		JPopupMenu popup = Menus.getPopupMenu();
		if (popup!=null) {
			add(popup);
			popup.show(this, x, y);
		}
	}

	public void mouseExited(MouseEvent e) {
		setCursor(defaultCursor);
		IJ.showStatus("");
	}


	public void mouseDragged(MouseEvent e) {
		int x = e.getX();
		int y = e.getY();
		xMouse = offScreenX(x);
		yMouse = offScreenY(y);
		flags = e.getModifiers();
		if (flags==0)  // workaround for Mac OS 9 bug
			flags = InputEvent.BUTTON1_MASK;
		if (Toolbar.getToolId()==Toolbar.HAND || IJ.spaceBarDown())
			;//scroll(x, y);
		else {
			IJ.setInputEvent(e);
			Roi roi = imp.getRoi();
			if (roi != null)
				roi.handleMouseDrag(x, y, flags);
		}
	}

	void handleRoiMouseDown(MouseEvent e) {
		int sx = e.getX();
		int sy = e.getY();
		int ox = offScreenX(sx);
		int oy = offScreenY(sy);
		Roi roi = imp.getRoi();
		int handle = roi!=null?roi.isHandle(sx, sy):-1;
		setRoiModState(e, roi, handle);
		if (roi!=null) {
			Rectangle r = roi.getBounds();
			int type = roi.getType();
			if (type==Roi.RECTANGLE && r.width==imp.getWidth() && r.height==imp.getHeight()
			&& roi.getPasteMode()==Roi.NOT_PASTING) {
				imp.killRoi();
				return;
			}
			if (handle>=0) {
				roi.mouseDownInHandle(handle, sx, sy);
				return;
			}
			if (roi.contains(ox, oy)) {
				if (roiModState==NO_MODS)
					roi.handleMouseDown(sx, sy);
				else {
					imp.killRoi();
					imp.createNewRoi(ox,oy);
				}
				return;
			}
			if ((type==Roi.POLYGON || type==Roi.POLYLINE || type==Roi.ANGLE)
			&& roi.getState()==roi.CONSTRUCTING)
				return;
		}
		imp.createNewRoi(ox,oy);
	}

	void setRoiModState(MouseEvent e, Roi roi, int handle) {
		if (roi==null || !IJ.isJava2())
			{roiModState = NO_MODS; return;}
		if (handle>=0 && roiModState==NO_MODS)
			return;
		if (roi.state==Roi.CONSTRUCTING)
			return;
		int tool = Toolbar.getToolId();
		if (tool>Toolbar.FREEROI && tool!=Toolbar.WAND)
			{roiModState = NO_MODS; return;}
		if (e.isShiftDown())
			roiModState = ADD_TO_ROI;
		else if (e.isAltDown())
			roiModState = SUBTRACT_FROM_ROI;
		else
			roiModState = NO_MODS;
		//IJ.log("setRoiModState: "+roiModState+" "+ (roi==null?"null":""+roi.state));
	}

	public void mouseReleased(MouseEvent e) {
		flags = e.getModifiers();
		flags &= ~InputEvent.BUTTON1_MASK; // make sure button 1 bit is not set
		//IJ.log("mouseReleased: "+flags);
		Roi roi = imp.getRoi();
		if (roi != null) {
			Rectangle r = roi.getBounds();
			if ((r.width==0 || r.height==0)
			&& !(roi.getType()==Roi.POLYGON || roi.getType()==Roi.POLYLINE || roi.getType()==Roi.ANGLE)
			&& !(roi instanceof TextRoi)
			&& roi.getState()==roi.CONSTRUCTING)
				imp.killRoi();
			else
				roi.handleMouseUp(e.getX(), e.getY());
		}
	}

	public void mouseMoved(MouseEvent e) {
		if (ij==null) return;
		int sx = e.getX();
		int sy = e.getY();
		int ox = offScreenX(sx);
		int oy = offScreenY(sy);
		flags = e.getModifiers();
		//if (IJ.debugMode) IJ.log(e.getX() + " " + e.getY() + " " + ox + " " + oy);
		setCursor(sx, sy, ox, oy);
		IJ.setInputEvent(e);
		Roi roi = imp.getRoi();
		if (roi!=null && (roi.getType()==Roi.POLYGON || roi.getType()==Roi.POLYLINE || roi.getType()==Roi.ANGLE)
		&& roi.getState()==roi.CONSTRUCTING) {
			PolygonRoi pRoi = (PolygonRoi)roi;
			pRoi.handleMouseMove(ox, oy);
		} else {
			IJ.showStatus("");
		}
		imp.mouseMoved(sx, sy);
	}

	public void mouseClicked(MouseEvent e) {}
	public void mouseEntered(MouseEvent e) {}


	static ImagePlus getClipboard() {
		return clipboard;
	}


	/** Copies the current ROI to the clipboard. The entire
		image is copied if there is no ROI. */
	public void copy(boolean cut) {
		Roi roi = imp.getRoi();
		String msg = (cut)?"Cut":"Copy";
		IJ.showStatus(msg+ "ing...");
		ImageProcessor ip = imp.getProcessor();
		ImageProcessor ip2 = ip.crop();
		clipboard = new ImagePlus("Clipboard", ip2);
		if (roi!=null && roi.getType()!=Roi.RECTANGLE)
			clipboard.setRoi((Roi)roi.clone());
		if (cut) {
			ip.snapshot();
			ip.setColor(Toolbar.getBackgroundColor());
			ip.fill();
			if (roi!=null && roi.getType()!=Roi.RECTANGLE)
				ip.reset(imp.getMask());
			imp.setColor(Toolbar.getForegroundColor());
			Undo.setup(Undo.FILTER, imp);
			imp.updateAndDraw();
		}
		int bytesPerPixel = 1;
		switch (clipboard.getType()) {
			case ImagePlus.GRAY16: bytesPerPixel = 2; break;
			case ImagePlus.GRAY32: case ImagePlus.COLOR_RGB: bytesPerPixel = 4;
		}
		IJ.showStatus(msg + ": " + (clipboard.getWidth()*clipboard.getHeight()*bytesPerPixel)/1024 + "k");
	}


	public void paste()
	{
		if (clipboard==null) {
			return;
		}
		int cType = clipboard.getType();
		int iType = imp.getType();

		boolean sameType = false;
		if ((cType==ImagePlus.GRAY8|cType==ImagePlus.COLOR_256)&&(iType==ImagePlus.GRAY8|iType==ImagePlus.COLOR_256)) sameType = true;
		else if ((cType==ImagePlus.COLOR_RGB|cType==ImagePlus.GRAY8|cType==ImagePlus.COLOR_256)&&iType==ImagePlus.COLOR_RGB) sameType = true;
		else if (cType==ImagePlus.GRAY16&&iType==ImagePlus.GRAY16) sameType = true;
		else if (cType==ImagePlus.GRAY32&&iType==ImagePlus.GRAY32) sameType = true;
		if (!sameType) {
			IJ.error("Images must be the same type to paste.");
			return;
		}
		int w = clipboard.getWidth();
		int h = clipboard.getHeight();
		if (w>imp.getWidth() || h>imp.getHeight()) {
			IJ.error("Image is too large to paste.");
			return;
		}
		Roi roi = imp.getRoi();
		Rectangle r = null;
		if (roi!=null)
			r = roi.getBounds();
		if (r==null || (r!=null && (w!=r.width || h!=r.height))) {
			// create a new roi centered on visible part of image
			Rectangle srcRect = getSrcRect();
			int xCenter = srcRect.x + srcRect.width/2;
			int yCenter = srcRect.y + srcRect.height/2;
			Roi cRoi = clipboard.getRoi();
			if (cRoi!=null && cRoi.getType()!=Roi.RECTANGLE) {
				cRoi.setImage(imp);
				cRoi.setLocation(xCenter-w/2, yCenter-h/2);
				imp.setRoi(cRoi);
			} else
				imp.setRoi(xCenter-w/2, yCenter-h/2, w, h);
			roi = imp.getRoi();
		}
		if (IJ.macroRunning()) {
			//non-interactive paste
			int pasteMode = Roi.getCurrentPasteMode();
			boolean nonRect = roi.getType()!=Roi.RECTANGLE;
			ImageProcessor ip = imp.getProcessor();
			if (nonRect) ip.snapshot();
			r = roi.getBounds();
			ip.copyBits(clipboard.getProcessor(), r.x, r.y, pasteMode);
			if (nonRect)
				ip.reset(imp.getMask());
			imp.updateAndDraw();
			imp.killRoi();
		} else {
			roi.startPaste(clipboard);
			Undo.setup(Undo.PASTE, imp);
		}
		imp.changes = true;
		//Image img = clipboard.getImage();
		//ImagePlus imp2 = new ImagePlus("Clipboard", img);
		//imp2.show();
	}


}