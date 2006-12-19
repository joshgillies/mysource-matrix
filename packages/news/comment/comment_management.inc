<?php
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
* $Id: comment_management.inc,v 1.2 2006/12/19 22:13:12 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Comment_Management
*
* @see Asset_Management
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Comment_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['name'] = Array(
								'added'		=> '0.1',
								'type'		=> 'text',
								'default'	=> '',
								'is_admin'      => false,
							  );

		$this->vars['comment'] = Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
									'is_admin'      => false,
								  );
	}

}//end class

?>
