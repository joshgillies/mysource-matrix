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
* $Id: bodycopy_table_management.inc,v 1.7 2006/12/05 05:45:17 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bodycopy_Table_Management
*
* @see Bodycopy_Container_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Table_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Bodycopy_Table_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'rows'	=> Array(
									'added'		=> '0.0.1',
									'type'		=> 'serialise',
									'default'	=> Array(),
									'update_fn'	=> '',
								   ),
					  );

	}//end constructor


}//end class

?>