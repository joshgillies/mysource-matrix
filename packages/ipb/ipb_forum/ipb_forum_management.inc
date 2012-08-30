<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_forum_management.inc,v 1.4 2012/08/30 00:58:55 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ipb_Forum_Management
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Forum_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ipb_Forum_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'description'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'root_forums'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'int',
												'default'	=> 0,
											   ),
						'topics'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'int',
												'default'	=> 0,
											   ),
						'last_post'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'last_poster_name'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
					  );

	}//end constructor


}//end class

?>