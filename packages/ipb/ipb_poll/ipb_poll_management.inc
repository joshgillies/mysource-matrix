<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ipb_poll_management.inc,v 1.1.1.1 2005/12/15 03:02:07 skim Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ipb_Poll_Management
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.1.1 $
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