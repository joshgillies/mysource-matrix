<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_poll_management.inc,v 1.2 2006/12/07 03:17:17 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ipb_Poll_Management
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.2 $
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