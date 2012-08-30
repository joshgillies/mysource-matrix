/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: View.java,v 1.4 2012/08/30 01:09:20 ewang Exp $
*
*/

package net.squiz.matrix.assetmap;

import net.squiz.matrix.matrixtree.*;
import javax.swing.*;

public interface View {
	public MatrixTree getTree();
	public String getName();
	public void setName(String name);
	public JComponent getViewComponent();
}
