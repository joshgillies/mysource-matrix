
package net.squiz.matrix.ui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import net.squiz.matrix.core.*;
import java.util.*;
import java.awt.image.*;

public class Spinner extends JComponent implements Runnable{

	public static final String SPINNER_ICON = "spinner.png";
	public static final int delay = 40;
	public static final int IMG_OFFSET = 15;

	private Dimension size = new Dimension(IMG_OFFSET, IMG_OFFSET);
	
	private boolean isStarted = false;
	private int framePtr = 0;
	private volatile Thread animator;
	private static Image[] frames;
	private static Image stopImage;
	
	public Spinner() {
		// we only need to do this once for all spinner instances
		if (frames == null) {
			ImageIcon spinner = (ImageIcon) GUIUtilities.getAssetMapIcon(SPINNER_ICON);
			Image spinnerImg = spinner.getImage();
			
			BufferedImage spinnerSrc = new BufferedImage(
				spinner.getIconWidth(),
				spinner.getIconHeight(),
				BufferedImage.TYPE_INT_ARGB_PRE
			);
	
			Graphics2D g2d = (Graphics2D) spinnerSrc.createGraphics();
			g2d.drawImage(spinnerImg, 0, 0, null);
			g2d.dispose();
			
			int numFrames = (spinner.getIconWidth() / IMG_OFFSET) - 1;
			int offset = IMG_OFFSET;
			frames = new Image[numFrames];
			
			stopImage = spinnerSrc.getSubimage(0, 0, IMG_OFFSET, IMG_OFFSET);
			
			for (int i = 0; i < numFrames; i++) {
				frames[i] = spinnerSrc.getSubimage(offset, 0, IMG_OFFSET, IMG_OFFSET);
				offset += IMG_OFFSET;
			}
		}
	}
	
	public void run() {
		Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
		long tm = System.currentTimeMillis();
		
		while (Thread.currentThread() == animator) {
			Graphics g = getGraphics();
			if (g != null)
				g.drawImage(frames[framePtr], 0, 0, null);
			if (framePtr == frames.length - 1)
				framePtr = 0;
			else
				framePtr++;

			try {
				tm += delay;
				Thread.sleep(Math.max(0, tm - System.currentTimeMillis()));
			} catch (InterruptedException e) {
				break;
			}
			Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		}
	}
	
	public Dimension getMaximumSize() {
		return size;
	}
	public Dimension getMinimumSize() {
		return size;
	}
	public Dimension getPreferredSize() {
		return size;
	}
	
	public void start() {
		isStarted = true;
		animator = new Thread(this);
		animator.start();
	}
	
	public void stop() {
		isStarted = false;
		framePtr = 0;
		animator = null;
		Graphics g = getGraphics();
		if (g != null)
			g.drawImage(stopImage, 0, 0, null);
	}
	
	public boolean isStarted() {
		return isStarted;
	}
}
