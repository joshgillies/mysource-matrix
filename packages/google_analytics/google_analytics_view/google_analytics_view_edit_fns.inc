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
* $Id: data_record_edit_fns.inc,v 1.4 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Data_Record_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_View_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor
	
	
	/**
	 * Paint the "overall" report.
	 *
	 */
	public function paintSummaryOverallReport($asset, &$o, $prefix)
	{
	    $months = 1;
	    $start_date = strtotime('-'.$months.' months 00:00:00');
	    $end_date   = strtotime('yesterday 00:00:00');
	    $day_count  = round(1 + (($end_date - $start_date) / 86400), 0);
	    $report = $asset->generateVisitorsReport(Array(), $start_date, $end_date);
	    
	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.date('jS M Y', $start_date).'</strong>',
	            '<strong>'.date('jS M Y', $end_date).'</strong>',
	            $day_count
	        );
	    $o->closeField();
	    
	    $o->openSection(translate('Sessions and users'));
	    $o->openRaw();
	    ?><table class="sq-backend-table" style="width: 60%">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="35%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Total sessions') ?></th>
	            <td><strong><?php echo number_format($report[0]['ga:sessions'], 0); ?></strong></td>
	            <td><?php echo translate('%s / day', number_format($report[0]['ga:sessions'] / $day_count, 0)); ?></td>
	            <td><?php echo translate(
	                '%s (%s%%) bounces',
	                number_format($report[0]['ga:bounces'], 0),
	                number_format((100.0 * $report[0]['ga:bounces'] / $report[0]['ga:sessions']), 1)
                ); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Unique users') ?></th>
	            <td><strong><?php echo number_format($report[0]['ga:users'], 0); ?></strong></td>
	            <td colspan="2"><?php echo translate('%s / day', number_format($report[0]['ga:users'] / $day_count, 0)); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('New users') ?></th>
	            <td colspan="3"><?php echo number_format($report[0]['ga:newUsers'], 0); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Sessions per user') ?></th>
	            <td colspan="3"><?php echo number_format(($report[0]['ga:sessions'] / $report[0]['ga:users']), 2); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Return visits per user') ?></th>
	            <td><?php echo number_format(($report[0]['ga:sessions'] / ($report[0]['ga:users'] - $report[0]['ga:newUsers']) - 1), 2); ?></td>
	            <td colspan="2"><?php echo translate('Excludes first visit from a new user during this period'); ?></td>
	        </tr>
	    </tbody></table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    $o->openSection(translate('Session statistics'));
	    $o->openRaw();
	    ?><table class="sq-backend-table" style="width: 60%">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="35%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Total page views') ?></th>
	            <td><strong><?php echo number_format($report[0]['ga:pageViews'], 0); ?></strong></td>
	            <td colspan="2"><?php echo translate('%s / day', number_format($report[0]['ga:pageViews'] / $day_count, 0)); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Page views per session') ?></th>
	            <td><?php echo number_format($report[0]['ga:pageViews'] / $report[0]['ga:sessions'], 1); ?></td>
	            <td colspan="2"><?php echo translate(
	                '%s excluding bounces',
	                number_format($report[0]['ga:pageViews'] / ($report[0]['ga:sessions'] - $report[0]['ga:bounces']), 1)
	            ); ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Session duration') ?></th>
	            <td><?php 
	            $time = $report[0]['ga:sessionDuration'] / $report[0]['ga:sessions'];
                    if ($time > 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), ($time % 60));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time, 1));
                    }
	            ?></td>
	            <td colspan="2"><?php
	                $nb_time = $report[0]['ga:sessionDuration'] / ($report[0]['ga:sessions'] - $report[0]['ga:bounces']);
                    if ($nb_time > 60) {
                        $nb_str = sprintf(translate('%s min %s sec', '%s', '%s'), (int)($nb_time / 60), ($nb_time % 60));
                    } else {
                        $nb_str = sprintf(translate('%s sec', '%s'), floor($nb_time, 1));
                    }
                        
                    echo translate('%s excluding bounces', $nb_str);
	            ?></td>
	        </tr>
	    </tbody>
	    </table><?php
	    $o->closeRaw();
	    $o->closeSection();
	    
	}//end paintSummaryOverallReport()


}//end class

?>
