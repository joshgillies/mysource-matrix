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
* $Id: page_custom_form_management.inc,v 1.12 2006/12/07 00:04:21 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Custom_Form_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Custom_Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'	=> Array (
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								   ),
					  );

	}//end constructor


}//end class
?>
