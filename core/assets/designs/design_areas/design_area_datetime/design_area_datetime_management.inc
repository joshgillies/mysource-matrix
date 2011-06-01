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
* $Id: design_area_datetime_management.inc,v 1.13 2008/05/14 01:47:47 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_DateTime_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_DateTime_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'format'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> 'h:ia  d/m/Y',
										'description'	=> 'Format of the datetime field, see PHP\'s date fn for format specifications <a href="http://www.php.net/date">http://www.php.net/date</a>',
									   ),
						'datetime'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'selection',
										'default'		=> 'now',
										'parameters'	=> Array(
															'options'		=> Array(
																				'now'		=> 'When the page is loaded',
																				'updated'	=> 'When the page was last modified',
																				'created'   => 'When the page was first created',
																				'published' => 'When the page was published',
																			   ),
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
														   ),
										'description'	=> 'The value to be used',
									   ),
					  );

	}//end constructor


}//end class
?>
