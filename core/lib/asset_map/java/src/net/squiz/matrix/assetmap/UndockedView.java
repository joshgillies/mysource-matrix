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
* $Id: UndockedView.java,v 1.4 2006/12/05 05:26:35 bcaldwell Exp $
*
*/

package net.squiz.matrix.assetmap;

import javax.swing.*;
import java.awt.event.*;
import net.squiz.matrix.matrixtree.*;

public class UndockedView extends JFrame implements View {

	private View view;
	private int index;

	public UndockedView(View view, int index) {
		this.view = view;
		this.index = index;

		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		setTitle(view.getName());
		getContentPane().add(view.getViewComponent());

		JSplitPane splitPane = ((BasicView)view).getSplitPane();
		splitPane.setDividerLocation(Integer.MAX_VALUE);
	}

	public MatrixTree getTree() {
		return view.getTree();
	}

	public View getInnerView() {
		return view;
	}

	public int getIndex() {
		return index;
	}

	public String getName() {
		return view.getName();
	}

	public void setName(String name) {
		view.setName(name);
	}

	public JComponent getViewComponent() {
		return view.getViewComponent();
	}
}
