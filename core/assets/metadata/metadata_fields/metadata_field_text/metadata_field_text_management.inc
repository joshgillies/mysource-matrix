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
* $Id: metadata_field_text_management.inc,v 1.9 2008/01/11 00:07:06 colivar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Text_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		// additional attrs., that were not defined in parent
		$this->vars = Array(
						'multi_line'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array('allow_empty' => FALSE),
										   ),
					  );

		$this->vars['char_limit'] = Array(
										'added'				=> '0.2',
										'type'				=> 'int',
										'default'			=> 0,
										'description'		=> 'character limit',
										'parameters'		=> Array('allow_negative'	=> FALSE),
									);

		$this->vars['box_width'] = Array(
										'added'				=> '0.2',
										'type'				=> 'int',
										'default'			=> 30,
										'description'		=> 'width of the box',
										'parameters'		=> Array('allow_negative'	=> FALSE, 'range_lower' => 1),
									);

		$this->vars['box_height'] = Array(
										'added'				=> '0.2',
										'type'				=> 'int',
										'default'			=> 7,
										'description'		=> 'height of the box',
										'parameters'		=> Array('allow_negative'	=> FALSE, 'range_lower' => 1),
									);

	}//end constructor


}//end class

?>