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
* $Id: Draggable.java,v 1.2 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.ui;

import java.awt.*;
import javax.swing.tree.*;

public interface Draggable {
	public Image getDragImage(TreePath[] path);
}
