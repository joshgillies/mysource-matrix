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
* $Id: FinderView.java,v 1.5 2006/12/05 05:26:35 bcaldwell Exp $
*
*/

package net.squiz.matrix.assetmap;

import javax.swing.*;
import javax.swing.tree.*;
import net.squiz.matrix.matrixtree.*;
import java.awt.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import javax.swing.plaf.*;
import net.squiz.matrix.plaf.*;

public class FinderView extends BasicView {

	protected FinderTree tree;

	public FinderView() {
		construct();
	}

	private void construct() {
		tree = (FinderTree)createFinderTree();
		JScrollPane scrollPane = new JScrollPane(tree);
		scrollPane.setBorder(BorderFactory.createEmptyBorder());

		setLayout(new BorderLayout());
		add(scrollPane);
		add(MatrixStatusBar.createStatusBar(), BorderLayout.SOUTH);
		add(new AssetMapMenuPanel(tree, false), BorderLayout.NORTH);
		setSize(300,500);
	}

}
