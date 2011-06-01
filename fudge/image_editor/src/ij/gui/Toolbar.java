package ij.gui;

import java.awt.*;
import java.awt.event.KeyEvent;
import java.awt.event.*;
import ij.*;
import ij.plugin.frame.Recorder;
import javax.swing.*;

/** The ImageJ toolbar. */
public class Toolbar extends JPanel implements MouseListener, MouseMotionListener {

	public static final int RECTANGLE = 0;
	public static final int OVAL = 1;
	public static final int POLYGON = 2;
	public static final int FREEROI = 3;
	public static final int LINE = 4;
	public static final int POLYLINE = 5;
	public static final int FREELINE = 6;
	public static final int WAND = 7;
	public static final int TEXT = 8;
	public static final int MAGNIFIER = 9;
	public static final int HAND = 10;
	public static final int DROPPER = 11;

	public static final int FIRST_SELECT_TOOL = 0;
	public static final int LAST_SELECT_TOOL = 8;

	// deprecated
	public static final int ANGLE = 999;
	public static final int CROSSHAIR = 999;


	public static final int NUM_TOOLS = 14;

	public static final int DIVIDER = 100;
	public static final int SPARE = 101;

	public static final int[] BUTTON_SEQUENCE = {
		RECTANGLE,
		OVAL,
		POLYGON,
		FREEROI,
		LINE,
		POLYLINE,
		FREELINE,
		WAND,
		DIVIDER,
		TEXT,
		DIVIDER,
		DROPPER,
		DIVIDER,
		MAGNIFIER
	};
		
	private static final int SIZE = 22;
	private static final int OFFSET = 3;
		
	private Dimension ps = new Dimension(SIZE*BUTTON_SEQUENCE.length, SIZE);
	private boolean[] down;
	private static int current;
	private int previous;
	private int x,y;
	private int xOffset, yOffset;
	private long mouseDownTime;
	private Graphics g;
	private static Toolbar instance;
	private int mpPrevious = RECTANGLE;
	private String[] names = new String[NUM_TOOLS];
	private String[] icons = new String[NUM_TOOLS];
	private int pc;
	private String icon;

	private static Color foregroundColor = Prefs.getColor(Prefs.FCOLOR,Color.black);
	private static Color backgroundColor = Prefs.getColor(Prefs.BCOLOR,Color.white);
	
	private Color gray = ImageJ.backgroundColor;
	private Color brighter = gray.brighter();
	private Color darker = gray.darker();
	private Color evenDarker = darker.darker();


	public Toolbar() {
		down = new boolean[NUM_TOOLS];
		resetButtons();
		down[0] = true;
		setForeground(foregroundColor);
		setBackground(gray);
		addMouseListener(this);
		addMouseMotionListener(this);
		instance = this;
	}

	/** Returns the ID of the current tool (Toolbar.RECTANGLE,
		Toolbar.OVAL, etc.). */
	public static int getToolId() {
		return current;
	}

	/** Returns a reference to the ImageJ toolbar. */
	public static Toolbar getInstance() {
		return instance;
	}

	private void drawButtons(Graphics g) {
		int currentOffset = 1;
		for (int i=0; i<BUTTON_SEQUENCE.length; i++) {
			drawButton(g, BUTTON_SEQUENCE[i], currentOffset);
			currentOffset += getButtonWidth(BUTTON_SEQUENCE[i]);
		}
	}

	private int getButtonWidth(int tool) {
		if (tool == DIVIDER) {
			return 11;
		} else {
			return 22;
		}
	}

	private void fill3DRect(Graphics g, int x, int y, int width, int height, boolean raised) {
		if (raised)
			g.setColor(gray);
		else
			g.setColor(darker);
		g.fillRect(x+1, y+1, width-2, height-2);
		g.setColor(raised ? brighter : evenDarker);
		g.drawLine(x, y, x, y + height - 1);
		g.drawLine(x + 1, y, x + width - 2, y);
		g.setColor(raised ? evenDarker : brighter);
		g.drawLine(x + 1, y + height - 1, x + width - 1, y + height - 1);
		g.drawLine(x + width - 1, y, x + width - 1, y + height - 2);
	}

	private void drawButton(Graphics g, int tool) {
		int i=0;
		int offset = 0;
		while ((i < BUTTON_SEQUENCE.length) && (BUTTON_SEQUENCE[i] != tool)) {
			offset += getButtonWidth(BUTTON_SEQUENCE[i]);
			i++;
		}
		drawButton(g, tool, offset);
	}


	private void drawButton(Graphics g, int tool, int currentOffset) {
		if (tool == DIVIDER) return;
        int index = tool;
        fill3DRect(g, currentOffset, 1, 22, 21, !down[tool]);
        g.setColor(Color.black);
        int x = currentOffset + 2;
		int y = OFFSET;
		if (down[tool]) { x++; y++;}
		this.g = g;
		if (icons[tool]!=null) {
			drawIcon(g, icons[tool], x, y);
			return;
		}
		switch (tool) {
			case RECTANGLE:
				g.drawRect(x+1, y+2, 14, 11);
				return;
			case OVAL:
				g.drawOval(x+1, y+3, 14, 11);
				return;
			case POLYGON:
				xOffset = x+1; yOffset = y+3;
				m(4,0); d(14,0); d(14,1); d(10,5); d(10,6);
				d(13,9); d(13,10); d(0,10); d(0,4); d(4,0);
				return;
			case FREEROI:
				xOffset = x+1; yOffset = y+3;
				m(3,0); d(5,0); d(7,2); d(9,2); d(11,0); d(13,0); d(14,1); d(15,2);
				d(15,4); d(14,5); d(14,6); d(12,8); d(11,8); d(10,9); d(9,9); d(8,10);
				d(5,10); d(3,8); d(2,8); d(1,7); d(1,6); d(0,5); d(0,2); d(1,1); d(2,1);
				return;
			case LINE:
				xOffset = x; yOffset = y+5;
				m(0,0); d(16,6);
				return;
			case POLYLINE:
				xOffset = x+1; yOffset = y+3;
				m(0,3); d(3,0); d(13,0); d(13,1); d(8,6); d(12,10);
				return;
			case FREELINE:
				xOffset = x+1; yOffset = y+4;
				m(0,1); d(2,3); d(4,3); d(7,0); d(8,0); d(10,4); d(14,8); d(15,8);
				return;
			case CROSSHAIR:
				xOffset = x; yOffset = y;
				m(1,8); d(6,8); d(6,6); d(10,6); d(10,10); d(6,10); d(6,9);
				m(8,1); d(8,5); m(11,8); d(15,8); m(8,11); d(8,15);
				m(8,8); d(8,8);
				return;
			case WAND:
				xOffset = x+2; yOffset = y+2;
				m(4,0); d(4,0);  m(2,0); d(3,1); d(4,2);  m(0,0); d(1,1);
				m(0,2); d(1,3); d(2,4);  m(0,4); d(0,4);  m(3,3); d(12,12);
				return;
			case TEXT:
				xOffset = x+2; yOffset = y+1;
				m(0,13); d(3,13);
				m(1,12); d(7,0); d(12,13);
				m(11,13); d(14,13);
				m(3,8); d(10,8);
				return;
			case MAGNIFIER:
				xOffset = x+2; yOffset = y+2;
				m(3,0); d(3,0); d(5,0); d(8,3); d(8,5); d(7,6); d(7,7);
				d(6,7); d(5,8); d(3,8); d(0,5); d(0,3); d(3,0);
				m(8,8); d(9,8); d(13,12); d(13,13); d(12,13); d(8,9); d(8,8);
				return;

			case HAND:
				xOffset = x+1; yOffset = y+1;
				m(5,14); d(2,11); d(2,10); d(0,8); d(0,7); d(1,6); d(2,6); d(4,8); 
				d(4,6); d(3,5); d(3,4); d(2,3); d(2,2); d(3,1); d(4,1); d(5,2); d(5,3);
				m(6,5); d(6,1); d(7,0); d(8,0); d(9,1); d(9,5);
				m(9,1); d(11,1); d(12,2); d(12,6);
				m(13,4); d(14,3); d(15,4); d(15,7); d(14,8);
				d(14,10); d(13,11); d(13,12); d(12,13); d(12,14);
				return;
			case DROPPER:
				xOffset = x; yOffset = y;
				g.setColor(foregroundColor);
				//m(0,0); d(17,0); d(17,17); d(0,17); d(0,0);
				m(12,2); d(14,2);
				m(11,3); d(15,3);
				m(11,4); d(15,4);
				m(8,5); d(15,5);
				m(9,6); d(14,6);
				m(10,7); d(12,7); d(12,9);
				m(8,7); d(2,13); d(2,15); d(4,15); d(11,8);
				g.setColor(backgroundColor);
				m(0,0); d(16,0); d(16,16); d(0,16); d(0,0);
				g.setColor(Color.black);
				return;
		}
	}
	
	void drawIcon(Graphics g, String icon, int x, int y) {
		//IJ.log("drawIcon: "+icon);
		this.icon = icon;
		int length = icon.length();
		int x1, y1, x2, y2;
		pc = 0;
		while (true) {
			char command = icon.charAt(pc++);
			if (pc>=length) break;
			switch (command) {
				case 'B': x+=v(); y+=v(); break;  // reset base
				case 'R': g.drawRect(x+v(), y+v(), v(), v()); break;  // rectangle
				case 'F': g.fillRect(x+v(), y+v(), v(), v()); break;  // filled rectangle
				case 'O': g.drawOval(x+v(), y+v(), v(), v()); break;  // oval
				case 'o': g.fillOval(x+v(), y+v(), v(), v()); break;  // filled oval
				case 'C': g.setColor(new Color(v()*16,v()*16,v()*16)); break; // set color
				case 'L': g.drawLine(x+v(), y+v(), x+v(), y+v()); break; // line
				case 'D': g.drawLine(x1=x+v(), x2=y+v(), x1, x2); break; // dot
				case 'P': // polyline
					x1=x+v(); y1=y+v();
					while (true) {
						x2=v(); if (x2==0) break;
						y2=v(); if (y2==0) break;
						x2+=x; y2+=y;
						g.drawLine(x1, y1, x2, y2);
						x1=x2; y1=y2;
					}
					break;
				case 'T': // text (one character)
					x2 = x+v();
					y2 = y+v();
					int size = v()*10+v();
					char[] c = new char[1];
					c[0] = pc<icon.length()?icon.charAt(pc++):'e';
					g.setFont(new Font("SansSerif", Font.BOLD, size));
					g.drawString(new String(c), x2, y2);
					break;
				default: break;
			}
			if (pc>=length) break;
		}
		g.setColor(Color.black);
	}
	
	int v() {
		if (pc>=icon.length()) return 0;
		char c = icon.charAt(pc++);
		//IJ.log("v: "+pc+" "+c+" "+toInt(c));
		switch (c) {
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			case 'a': return 10;
			case 'b': return 11;
			case 'c': return 12;
			case 'd': return 13;
			case 'e': return 14;
			case 'f': return 15;
			default: return 0;
		}
	}
	
	private void showMessage(int tool) {
		if (tool == -1) return;
		if ((tool < names.length) && (tool >= 0) && (names[tool]!=null)) {
			IJ.showStatus(names[tool]);
			return;
		}
		switch (tool) {
			case RECTANGLE:
				IJ.showStatus("Rectangular selection tool");
				return;
			case OVAL:
				IJ.showStatus("Oval selection tool");
				return;
			case POLYGON:
				IJ.showStatus("Polygon selection tool");
				return;
			case FREEROI:
				IJ.showStatus("Freehand selection tool");
				return;
			case LINE:
				IJ.showStatus("Straight line selection tool");
				return;
			case POLYLINE:
				IJ.showStatus("Segmented line selection tool");
				return;
			case FREELINE:
				IJ.showStatus("Freehand line selection tool");
				return;
			case CROSSHAIR:
				IJ.showStatus("Crosshair (mark and count) tool");
				return;
			case WAND:
				IJ.showStatus("Wand (tracing) tool: click an object to select it");
				return;
			case TEXT:
				IJ.showStatus("Text tool: click and drag to create a text box");
				return;
			case MAGNIFIER:
				IJ.showStatus("Zoom Tool: left-click to zoom in, right-click to zoom out");
				return;
			case HAND:
				IJ.showStatus("Scrolling tool");
				return;
			case DROPPER:
				IJ.showStatus("Color picker (" + foregroundColor.getRed() + ","
				+ foregroundColor.getGreen() + "," + foregroundColor.getBlue() + ")");
				return;
			default:
				IJ.showStatus("");
				return;
		}
	}

	private void m(int x, int y) {
		this.x = xOffset+x;
		this.y = yOffset+y;
	}

	private void d(int x, int y) {
		x += xOffset;
		y += yOffset;
		g.drawLine(this.x, this.y, x, y);
		this.x = x;
		this.y = y;
	}

	private void resetButtons() {
		for (int i=0; i<NUM_TOOLS; i++)
			down[i] = false;
	}

	public void paint(Graphics g) {
		drawButtons(g);
	}

	public void setTool(int tool) {
		if (tool==current || tool<0 || tool>=NUM_TOOLS) {
			return;
		}
		if ((FIRST_SELECT_TOOL <= tool) && (LAST_SELECT_TOOL >= tool)) {
			IJ.getInstance().getImagePlus().killRoi();
		}
		current = tool;
		down[current] = true;
		down[previous] = false;
		Graphics g = this.getGraphics();
		drawButton(g, previous);
		drawButton(g, current);
		g.dispose();
		showMessage(current);
		previous = current;
		if (IJ.isMacOSX())
			repaint();
	}

	public static Color getForegroundColor() {
		return foregroundColor;
	}

	public static void setForegroundColor(Color c) {
		if (c!=null) {
			foregroundColor = c;
			updateColors();
		}
	}

	public static Color getBackgroundColor() {
		return backgroundColor;
	}

	public static void setBackgroundColor(Color c) {
		if (c!=null) {
			backgroundColor = c;
			updateColors();
		}
	}
	
	static void updateColors() {
		Toolbar tb = getInstance();
		Graphics g = tb.getGraphics();
		tb.drawButton(g, DROPPER);
		g.dispose();
	}

	public void mousePressed(MouseEvent e) {
		int x = e.getX();
		int newTool = getToolFromCoord(x);
		if (newTool == -1) return;
		
		boolean doubleClick = newTool==current && (System.currentTimeMillis()-mouseDownTime)<=500;
 		mouseDownTime = System.currentTimeMillis();
		if (!doubleClick) {
			mpPrevious = current;
			setTool(newTool);
		} else {
			ImagePlus imp = IJ.getInstance().getImagePlus();
			switch (current) {
				case FREEROI:
					IJ.doCommand("Set Measurements...");
					setTool(mpPrevious);
					break;
				case MAGNIFIER:
					if (imp!=null) imp.getCanvas().unzoom();
					break;
				case POLYGON:
					if (imp!=null) IJ.doCommand("Calibrate...");
					setTool(mpPrevious);
					break;
				case LINE: case POLYLINE: case FREELINE:
					IJ.doCommand("Line Width...");
					break;
				case CROSSHAIR:
					IJ.doCommand("Crosshair...");
					break;
				case TEXT:
					IJ.doCommand("Fonts...");
					break;
				case DROPPER:
					IJ.doCommand("Color Picker...");
					setTool(mpPrevious);
					break;
				default:
			}
		}
	}
	
	public void mouseReleased(MouseEvent e) {}
	public void mouseExited(MouseEvent e) { IJ.showStatus(""); }
	public void mouseClicked(MouseEvent e) {}
	public void mouseEntered(MouseEvent e) {}
    public void mouseDragged(MouseEvent e) {}
	
	public Dimension getPreferredSize(){
		return ps;
	}

	public Dimension getMinimumSize(){
		return ps;
	}
	
	public void mouseMoved(MouseEvent e) {
		int x = e.getX();
		showMessage(getToolFromCoord(x));
	}

	private int getToolFromCoord(int x) {
		int offset = 0;
		int i=0;
		while ((i != BUTTON_SEQUENCE.length) && (offset < x)) {
			offset += getButtonWidth(BUTTON_SEQUENCE[i]);
			i++;
		}
		return BUTTON_SEQUENCE[i-1];
	}

	

}
