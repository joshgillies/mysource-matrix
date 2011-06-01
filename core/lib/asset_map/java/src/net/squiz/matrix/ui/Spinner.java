/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: Spinner.java,v 1.4 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.ui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import net.squiz.matrix.core.*;
import java.util.*;
import java.awt.image.*;

/**
 * A spinner primvides an animation to indicate that an operation in the system
 * is currently progress. When not spinner, the spinner is in a stopped state
 * and shows a stopped icon to indicate that no operation is in progress.
 * The spinner image itself is a sequence of frames; the first being the stopped
 * image and those preceeding it being the animation steps of the spinner.
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class Spinner extends JComponent implements Runnable {

	/** The spinner icon */
	public static final String SPINNER_ICON = "spinner.png";
	/** The delay between each animation operation */
	public static final int delay = 40;
	/** The offset which each aniation frame occurs in the image */
	public static final int IMG_OFFSET = 15;

	private Dimension size = new Dimension(IMG_OFFSET, IMG_OFFSET);

	private boolean isStarted = false;
	private int framePtr = 0;
	private volatile Thread animator;
	private Image[] frames;
	private Image stopImage;

	/**
	 * Constructs a spinner object.
	 * @return the spinner object
	 */
	public Spinner() {
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

		// the number of frames of animation is one less than the icon width
		// as we need to remove the stop spinner state from the frames
		int numFrames = (spinner.getIconWidth() / IMG_OFFSET) - 1;
		int offset = IMG_OFFSET;
		frames = new Image[numFrames];

		// the stop image is the first image square in the main spinner sequence
		stopImage = spinnerSrc.getSubimage(0, 0, IMG_OFFSET, IMG_OFFSET);

		// all other images are IMG_OFFSET from the stop image
		for (int i = 0; i < numFrames; i++) {
			frames[i] = spinnerSrc.getSubimage(offset, 0, IMG_OFFSET, IMG_OFFSET);
			offset += IMG_OFFSET;
		}
	}

	/**
	 * The run method of this Thread.
	 */
	public void run() {
		Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
		long tm = System.currentTimeMillis();

		while (Thread.currentThread() == animator) {
			Graphics g = getGraphics();
			if (g != null) {
				g.drawImage(frames[framePtr], 0, 0, null);
			}
			if (framePtr == frames.length - 1) {
				framePtr = 0;
			} else {
				framePtr++;
			}

			try {
				tm += delay;
				Thread.sleep(Math.max(0, tm - System.currentTimeMillis()));
			} catch (InterruptedException e) {
				break;
			}
			Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		}
	}

	/**
	 * Paints the component.
	 * there are no components to paint so we explicity paint the stopped
	 * spinner state when paint is called.
	 * @param g the graphics that we are paintint to
	 */
	public void paint(Graphics g) {
		super.paint(g);
		// if we are not spinning we need to paint the stop spinner state
		// whenever paint gets called
		if (!isStarted) {
			g.drawImage(stopImage, 0, 0, null);
		}
	}

	/**
	 * Returns the maximum size of the spinner object
	 * @return the maximum size
	 */
	public Dimension getMaximumSize() {
		return size;
	}

	/**
	 * Returns the maximum size of the spinner object
	 * @return the minimum size
	 */
	public Dimension getMinimumSize() {
		return size;
	}

	/**
	 * Returns the preferred size of the spinner object
	 * @return the preferred size
	 */
	public Dimension getPreferredSize() {
		return size;
	}

	/**
	 * Starts the animation of the spinner
	 * @see stop()
	 * @see isStarted()
	 */
	public void start() {
		if (isStarted)
			return;
		isStarted = true;
		animator = new Thread(this);
		animator.start();
	}

	/**
	 * Stops the animation of the spinner
	 * @see start()
	 * @see isStarted()
	 */
	public void stop() {
		isStarted = false;
		framePtr = 0;
		animator = null;
		Graphics g = getGraphics();
		if (g != null)
			g.drawImage(stopImage, 0, 0, null);
	}

	/**
	 * Returns TRUE is the spinner is current animating
	 * @return TRUE is the spinner is current animating
	 * @see start()
	 * @see stop()
	 */
	public boolean isStarted() {
		return isStarted;
	}
}
