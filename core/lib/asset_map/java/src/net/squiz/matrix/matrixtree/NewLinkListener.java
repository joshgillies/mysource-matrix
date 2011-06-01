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
* $Id: NewLinkListener.java,v 1.2 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.matrixtree;
import java.util.EventListener;

public interface NewLinkListener extends EventListener {
	public void requestForNewLink(NewLinkEvent evt);
}
