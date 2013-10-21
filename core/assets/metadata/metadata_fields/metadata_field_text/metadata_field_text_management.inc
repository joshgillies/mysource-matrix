<?php
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
* $Id: metadata_field_text_management.inc,v 1.10.10.1 2013/10/21 03:41:55 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.10.10.1 $
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

		$this->vars['char_limit'] 	= Array(
										'added'				=> '0.2',
										'type'				=> 'int',
										'default'			=> 0,
										'description'		=> 'character limit',
										'parameters'		=> Array('allow_negative'	=> FALSE),
									);

		$this->vars['box_width']	= Array(
										'added'				=> '0.2',
										'type'				=> 'int',
										'default'			=> 30,
										'description'		=> 'width of the box',
										'parameters'		=> Array('allow_negative'	=> FALSE, 'range_lower' => 1),
									);

		$this->vars['box_height']	= Array(
										'added'				=> '0.2',
										'type'				=> 'int',
										'default'			=> 7,
										'description'		=> 'height of the box',
										'parameters'		=> Array('allow_negative'	=> FALSE, 'range_lower' => 1),
									);
		$this->vars['field_name_text']= Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> 'name',
											'is_contextable'=> TRUE,
										   );
		$this->vars['value_name_text'] = Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> 'content',
											'is_contextable'=> TRUE,
										   );

	}//end constructor


}//end class

?>
