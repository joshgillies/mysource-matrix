<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: weatherco_feed_management.inc,v 1.1 2007/10/24 05:46:29 bshkara Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Weatherco_Feed_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/

class Weatherco_Feed_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Weatherco_Feed_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars	= Array (
						'password'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'password',
										'default'		=> '',
										'description'	=> 'The password provided by The Weather Company upon registration',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
														   ),
									   ),
						'userid'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The user ID provided by The Weather Company upon signup',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
														   ),
									   ),
					  );

	}//end constructor


}//end class
?>
