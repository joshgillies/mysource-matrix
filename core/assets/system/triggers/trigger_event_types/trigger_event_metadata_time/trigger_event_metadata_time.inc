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
* $Id: trigger_event_metadata_time.inc,v 1.2 2012/08/30 01:09:16 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_event/trigger_event.inc';

/**
* Trigger_Event_Metadata_Time
*
* Fired by the Cron Job Metadata Trigger when a metadata field date is expired
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Event_Metadata_Time extends Trigger_Event
{

}//end class

?>