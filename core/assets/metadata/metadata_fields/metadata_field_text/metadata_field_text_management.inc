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
* $Id: metadata_field_text_management.inc,v 1.7 2006/12/06 01:07:36 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Text_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Text_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// additional attrs., that were not defined in parent
		$this->vars = Array(
						'multi_line'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array('allow_empty' => FALSE),
										   ),
					  );

	}//end constructor


}//end class

?>