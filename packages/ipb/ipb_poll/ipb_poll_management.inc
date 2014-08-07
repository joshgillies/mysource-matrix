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
* $Id: ipb_poll_management.inc,v 1.4 2012/08/30 00:58:55 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ipb_Poll_Management
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Poll_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ipb_Poll_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'pid'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'tid'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'start_date'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'choices'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'starter_id'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'votes'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'forum_id'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										   ),
						'poll_question'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'poll_state'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
					  );

	}//end constructor


}//end class

?>