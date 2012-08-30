<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: weatherco_feed_management.inc,v 1.3 2012/08/30 00:58:16 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Weatherco_Feed_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.3 $
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
