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
* $Id: MatrixTabbedPane.java,v 1.6 2007/04/19 06:28:40 rong Exp $
*
*/

package net.squiz.matrix.assetmap;

import net.squiz.matrix.ui.*;
import net.squiz.matrix.core.*;
import javax.swing.tree.TreePath;
import net.squiz.matrix.matrixtree.*;

import javax.swing.*;
import javax.swing.event.*;

import java.awt.event.*;
import java.awt.*;
import java.awt.dnd.*;

public class MatrixTabbedPane extends VerticalTabbedPane {

	private javax.swing.Timer tabChangerTimer;
	public final Redocker REDOCK_HANDLER = new Redocker();
	private boolean allUndocked = false;

	public MatrixTabbedPane(int alignment) {
		super(alignment);

		DropHandler dropHandler = new DropHandler();
		DropTarget dropTarget = new DropTarget(this, dropHandler);

		MouseListener listener = new MouseAdapter() {
			public void mouseClicked(MouseEvent evt) {
				if (evt.getClickCount() == 2) {
					JTabbedPane source = (JTabbedPane) evt.getSource();
					View view = (View) source.getSelectedComponent();
					if (view != null) {
						// dont do anything if the click is not within the
						// bounds of a tab
						if (indexAtLocation(evt.getX(), evt.getY()) == -1)
							return;

						JPanel panel = new JPanel();
						panel.setBackground(Color.WHITE);
						JLabel tabsUndocked = new JLabel("", GUIUtilities.getAssetMapIcon("matrix_logo.png"), CENTER);
						tabsUndocked.setVerticalTextPosition(JLabel.CENTER);
						tabsUndocked.setHorizontalTextPosition(JLabel.CENTER);
						tabsUndocked.setVerticalAlignment(JLabel.CENTER);
						tabsUndocked.setHorizontalAlignment(JLabel.CENTER);
						tabsUndocked.setBackground(Color.WHITE);

						int fillerWidth = getWidth();
						int fillerHeight = ( getHeight() / 3 ) ;
						Dimension filler = new Dimension(fillerWidth,fillerHeight);

						panel.add(new Box.Filler(filler,filler,filler), BorderLayout.NORTH);
						panel.add(tabsUndocked, BorderLayout.CENTER);
						source.setComponentAt(source.getSelectedIndex(), panel);

						UndockedView udView = new UndockedView(view, source.getSelectedIndex());
						AssetMap.applet.addKeyAndContainerListenerRecursively(udView);

						udView.setSize(300, 600);
						GUIUtilities.showInScreenCenter(udView);
						udView.toFront();
						udView.setExtendedState(Frame.NORMAL);
						udView.addWindowListener(REDOCK_HANDLER);
						source.setEnabledAt(source.getSelectedIndex(), false);

						for (int i = 0; i < source.getTabCount(); i++) {
							if (i != source.getSelectedIndex() && source.isEnabledAt(i)) {
								source.setSelectedIndex(i);
								allUndocked = false;
								break;
							}

							if (i == source.getTabCount() - 1) {
								source.setSelectedIndex(-1);
								allUndocked = true;
							}
						}
					}
				}
			}
		};

		addMouseListener(listener);
		addChangeListener(new CueTransferHandler());
	}

	public void addView(String name, View view) {
		view.setName(name);
		addTab(name, GUIUtilities.getAssetMapIcon("tree.png"), view.getViewComponent());
	}

	public boolean isAllUndocked() {
		return allUndocked ? true : false;
	}


	class Redocker extends WindowAdapter {
		public void windowClosing(WindowEvent evt) {
			UndockedView view = (UndockedView) evt.getComponent();
			JComponent basicView = view.getViewComponent();

			JSplitPane splitPane = ((BasicView)basicView).getSplitPane();
			splitPane.setDividerLocation(Integer.MAX_VALUE);

			for (int i = 0; i < getTabCount(); i++) {
				// make the other disabled tab a basic view
				// so that tabbedpane does not have problem to paint
				if (isEnabledAt(i) == false && i != view.getIndex()) {
					BasicView newView = new BasicView();
					setComponentAt(i, newView);
				}
			}

			setComponentAt(view.getIndex(), basicView);
			setEnabledAt(view.getIndex(), true);
			setSelectedIndex(view.getIndex());
			allUndocked = false;
		}
	}

	protected class CueTransferHandler implements ChangeListener {

		public void stateChanged(ChangeEvent evt) {

			try {
				JTabbedPane pane = (JTabbedPane) evt.getSource();
				BasicView view = (BasicView) pane.getComponentAt(pane.getSelectedIndex());
				MatrixTree currentTree = view.getTree();

				for (int i = 0; i < pane.getTabCount(); i++) {
					// it might be a JPanel so we need to check to see that its a BasicView
					if (!(pane.getComponentAt(i) instanceof BasicView))
						continue;
					MatrixTree tree = ( (BasicView) ( pane.getComponentAt(i) ) ).getTree();
					TreePath[] path = new TreePath[1];
					path[0] = tree.getCuePath();
					if (path[0] != null) {
						currentTree.startCueMode(path);
						tree.stopCueMode();
						break;
					}
				}
			} catch (ArrayIndexOutOfBoundsException ex) {}
		}
	}

	protected class DropHandler implements DropTargetListener {

		private javax.swing.Timer tabChangerTimer;
		private Point lastMousePt;

		public DropHandler() {
			ActionListener timerListener = new ActionListener() {
				public void actionPerformed(ActionEvent evt) {
					setSelectedIndex(indexAtLocation(lastMousePt.x, lastMousePt.y));
				}
			};
			tabChangerTimer = new javax.swing.Timer(1000, timerListener);
			tabChangerTimer.setRepeats(false);
		}

		public void dragOver(DropTargetDragEvent dtde) {
			lastMousePt = dtde.getLocation();
			if (indexAtLocation(lastMousePt.x, lastMousePt.y) != -1) {
				if (!tabChangerTimer.isRunning()) {
					tabChangerTimer.start();
				}
			} else {
				if (tabChangerTimer.isRunning())
					tabChangerTimer.stop();
			}
		}

		public void dragEnter(DropTargetDragEvent dtde) {}
		public void dragExit(DropTargetEvent dte) {}
		public void drop(DropTargetDropEvent dtde) {}
		public void dropActionChanged(DropTargetDragEvent dtde) {}

	}//end class DrapHandler
}
