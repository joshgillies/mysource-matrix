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
* $Id: View.java,v 1.3 2006/12/05 05:26:35 bcaldwell Exp $
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
