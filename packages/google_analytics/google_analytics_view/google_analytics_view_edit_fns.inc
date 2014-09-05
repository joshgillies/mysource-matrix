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


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

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
class Google_Analytics_View_Edit_Fns extends Page_Edit_Fns
{

    /**
     * Not set text from GA.
     *
     * @const string.
     */
    const NOT_SET = '(not set)';

    /**
     * Not provided text from GA.
     *
     * @const string.
     */
    const NOT_PROVIDED = '(not provided)';


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
	    $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'google_analytics_connector', FALSE, NULL, 'minor');
	    $connector      = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['majorid']);
        $period         = 'P1M';

	    $end_date   = new DateTime('today 00:00:00');
	    $start_date = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff  = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));
	    $day_count  = (int) $date_diff->format('%a');

	    $prev_end_date = clone $end_date;
	    $prev_end_date->sub(new DateInterval('P'.$day_count.'D'));

	    $prev_start_date = clone $start_date;
	    $prev_start_date->sub(new DateInterval('P'.$day_count.'D'));

	    $report = $asset->generateReport(
	        Array(),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $report = array_get_index($report['rows'], 0, Array());

	    $previous_report = $asset->generateReport(
	        Array(),
	        Google_Analytics_View::SITE_METRICS,
	        $prev_start_date->format('Y-m-d'),
	        $prev_end_date->format('Y-m-d')
	    );

	    $previous_report = array_get_index($previous_report['rows'], 0, Array());

	    $visTypeRawReport = $asset->generateReport(
	        Array('ga:userType'),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    foreach ($visTypeRawReport['rows'] as $row) {
	        if ($row['ga:userType'] === 'New Visitor') {
	            $visTypeReport['new'] = $row;
	        } if ($row['ga:userType'] === 'Returning Visitor') {
	            $visTypeReport['returning'] = $row;
	        }
	    }//end foreach

	    $visTypeReport['new']       = array_get_index($visTypeReport, 'new', Array());
	    $visTypeReport['returning'] = array_get_index($visTypeReport, 'returning', Array());
	    unset($visTypeRawReport);

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $day_count
	        );
	    $o->closeField();

	    // Sessions and unique users.
	    $o->openSection(translate('Sessions and page views'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="35%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Total sessions') ?></th>
	            <td><strong><?php echo number_format($report['ga:sessions'], 0); ?></strong></td>
	            <td><?php echo translate('%s / day', number_format($report['ga:sessions'] / $day_count, 0)); ?></td>
	            <td><?php
	            if ($report['ga:sessions'] === $previous_report['ga:sessions']) {
	                echo translate('Steady');
	            } else if ($previous_report['ga:sessions'] === 0) {
	                echo translate('Increased from zero');
	            } else if ($report['ga:sessions'] > $previous_report['ga:sessions']) {
	                echo sprintf(translate('Increase of %s%%%%', '%s'), number_format(100 * (($report['ga:sessions'] / $previous_report['ga:sessions']) - 1), 1));
	            } else {
	                echo sprintf(translate('Decrease of %s%%%%', '%s'), number_format(100 * (1 - ($report['ga:sessions'] / $previous_report['ga:sessions'])), 1));
	            }
	            ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Total page views') ?></th>
	            <td><strong><?php echo number_format($report['ga:pageviews'], 0); ?></strong></td>
	            <td><?php echo translate('%s / day', number_format($report['ga:pageviews'] / $day_count, 0)); ?></td>
	            <td><?php
	            if ($report['ga:pageviews'] === $previous_report['ga:pageviews']) {
	                echo translate('Steady');
	            } else if ($previous_report['ga:pageviews'] === 0) {
	                echo translate('Increased from zero');
	            } else if ($report['ga:pageviews'] > $previous_report['ga:pageviews']) {
	                echo sprintf(translate('Increase of %s%%%%', '%s'), number_format(100 * (($report['ga:pageviews'] / $previous_report['ga:pageviews']) - 1), 1));
	            } else {
	                echo sprintf(translate('Decrease of %s%%%%', '%s'), number_format(100 * (1 - ($report['ga:pageviews'] / $previous_report['ga:pageviews'])), 1));
	            }
	            ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Bounce rate') ?></th>
	            <td><strong><?php echo number_format((100.0 * $report['ga:bounces'] / $report['ga:sessions']), 2) ?>%</strong> (<?php echo number_format($report['ga:bounces'], 0).')'; ?></td>
	            <td></td>
	            <td><?php
	            if (($previous_report['ga:sessions'] > 0) && ($report['ga:sessions'] > 0)) {
                    $bounce_rate      = (100.0 * $report['ga:bounces'] / $report['ga:sessions']);
                    $prev_bounce_rate = (100.0 * $previous_report['ga:bounces'] / $previous_report['ga:sessions']);
                    if ($bounce_rate === $prev_bounce_rate) {
                        echo translate('Steady');
                    } else if ($prev_bounce_rate === 0) {
                        echo translate('Increased from zero');
                    } else if ($bounce_rate > $prev_bounce_rate) {
                        echo sprintf(translate('Increase of %s%%%%', '%s'), number_format(100 * (($bounce_rate / $prev_bounce_rate) - 1), 1));
                    } else {
                        echo sprintf(translate('Decrease of %s%%%%', '%s'), number_format(100 * (1 - ($bounce_rate / $prev_bounce_rate)), 1));
                    }
	            }
	            ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Session duration') ?></th>
	            <td><strong><?php
	                $time = $report['ga:sessionDuration'] / $report['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
	            ?></strong></td>
	            <td></td>
	            <td><?php
	            if (($previous_report['ga:sessions'] > 0) && ($report['ga:sessions'] > 0)) {
                    $time      = $report['ga:sessionDuration'] / $report['ga:sessions'];
                    $prev_time = $previous_report['ga:sessionDuration'] / $previous_report['ga:sessions'];
                    if ($time === $prev_time) {
                        echo translate('Steady');
                    } else if ($prev_time === 0) {
                        echo translate('Increased from zero');
                    } else if ($time > $prev_time) {
                        echo sprintf(translate('Increase of %s%%%%', '%s'), number_format(100 * (($time / $prev_time) - 1), 1));
                    } else {
                        echo sprintf(translate('Decrease of %s%%%%', '%s'), number_format(100 * (1 - ($time / $prev_time)), 1));
                    }
	            }
	            ?></td>
	        </tr>
	        <tr>
	            <th><?php echo translate('Average page speed') ?></th>
	            <td><strong><?php
	                if ($report['ga:pageLoadSample'] === 0) {
	                    echo translate('N/A');
	                } else {
	                    $time = $report['ga:pageLoadTime'] / $report['ga:pageLoadSample'];
	                    echo sprintf(translate('%s sec', '%s'), number_format($time / 1000, 2));
	                }
	            ?></strong></td>
	            <td><?php
	                echo sprintf(translate('Sample of %s', '%s'), number_format($report['ga:pageLoadSample']));
	            ?></td>
	            <td><?php
	            if (($previous_report['ga:pageLoadSample'] > 0) && ($report['ga:pageLoadSample'] > 0)) {
                    $time      = $report['ga:pageLoadTime'] / $report['ga:pageLoadSample'];
                    $prev_time = $previous_report['ga:pageLoadTime'] / $previous_report['ga:pageLoadSample'];
                    if ($time === $prev_time) {
                        echo translate('Steady');
                    } else if ($prev_time === 0) {
                        echo translate('Increased from zero');
                    } else if ($time > $prev_time) {
                        echo sprintf(translate('Increase of %s%%%%', '%s'), number_format(100 * (($time / $prev_time) - 1), 1));
                    } else {
                        echo sprintf(translate('Decrease of %s%%%%', '%s'), number_format(100 * (1 - ($time / $prev_time)), 1));
                    }
	            }
	            ?></td>
	        </tr>
	    </tbody></table><?php
	    $o->closeRaw();
	    $o->closeSection();

	    // Sessions and unique users.
	    $o->openSection(translate('Top external referrers'));
	    $o->openRaw();
	    $this->_paintExternalReferrerSummary($asset, $o, $prefix, 10);
	    $o->closeRaw();
	    $o->closeSection();


	    // Sessions and unique users.
	    $o->openSection(translate('Top campaigns'));
	    $o->openRaw();
	    $this->_paintTopCampaigns($asset, $o, $prefix, 10);
	    $o->closeRaw();
	    $o->closeSection();

	    return FALSE;

	}//end paintSummaryOverallReport()


	/**
	 * Paint a Top Campaigns report.
	 *
	 */
	public function _paintExternalReferrerSummary($asset, &$o, $prefix, $limit=5)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the report.
	    $raw_report = $asset->generateReport(
	        Array(
	            'ga:medium',
	            'ga:source',
            ),
            Array(
                'ga:sessions',
                'ga:pageviews',
                'ga:sessionDuration',
                'ga:bounces'
            ),
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $report = $raw_report['rows'];
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    $total_sessions = array_reduce($report, function($carry, $row) {
            return $carry + $row['ga:sessions'];
	    }, 0);

	    ?><table class="sq-backend-table">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Medium / Source') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Page Views') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	            <th><?php echo translate('Bounce Rate') ?></td>
	        </tr><?php
	        $rank = 0;
            for ($i = 0; ($i < count($report) && ($rank < $limit)); $i++) {
	            $row =& $report[$i];
	            if ($row['ga:medium'] !== 'referral') {
	                continue;
	            }

	            $rank++;
            ?><tr>
                <td><?php
                echo $rank.'. '.$row['ga:source'];
                ?></td>
                <td style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <td style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:pageviews'], 0); ?></th>
                <td style="text-align: right"><?php
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:bounces'] / $row['ga:sessions'] * 100.0, 2).'%'; ?></th>
                </tr><?php
            }//end foreach
            ?></tbody></table><?php

	    return FALSE;

	}//end _paintExternalReferrerSummary()


	/**
	 * Paint a Top Campaigns report.
	 *
	 */
	public function _paintTopCampaigns($asset, &$o, $prefix, $limit=5)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the report.
	    $raw_report = $asset->generateReport(
	        Array(
	            'ga:source',
	            'ga:medium',
            ),
            Array(
                'ga:sessions',
                'ga:pageviews',
                'ga:sessionDuration',
                'ga:bounces'
            ),
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $report = $raw_report['rows'];
	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    $total_sessions = array_reduce($report, function($carry, $row) {
            return $carry + $row['ga:sessions'];
	    }, 0);

	    ?><table class="sq-backend-table">
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Medium / Source') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Page Views') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	            <th><?php echo translate('Bounce Rate') ?></td>
	        </tr><?php
	        $rank = 0;
            for ($i = 0; ($i < count($report) && ($rank < $limit)); $i++) {
	            $row =& $report[$i];
	            if (($row['ga:medium'] === '(none)') || ($row['ga:medium'] === 'referral')) {
	                continue;
	            } else if ($row['ga:medium'] === 'cpc') {
	                $row['ga:medium'] = translate('Paid search');
	                $row['ga:source'] = ucwords($row['ga:source']);
	            } else if ($row['ga:medium'] == 'organic') {
	                $row['ga:medium'] = translate('Search');
	                $row['ga:source'] = ucwords($row['ga:source']);
	            }

	            $rank++;
            ?><tr>
                <td><?php
                echo $rank.'. '.ucfirst($row['ga:medium']).' / '.$row['ga:source'];
                ?></td>
                <td style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <td style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:pageviews'], 0); ?></th>
                <td style="text-align: right"><?php
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:bounces'] / $row['ga:sessions'] * 100.0, 2).'%'; ?></th>
                </tr><?php
            }//end foreach
            ?></tbody></table><?php

	    return FALSE;

	}//end _paintTopCampaigns()


	/**
	 * Paint the "Goals" report.
	 *
	 */
	public function paintGoalsReport($asset, &$o, $prefix)
	{
	    $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'google_analytics_connector', FALSE, NULL, 'minor');
	    $connector      = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['majorid']);
        $period         = 'P1M';

	    $end_date   = new DateTime('today 00:00:00');
	    $start_date = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff  = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));
	    $day_count  = (int) $date_diff->format('%a');

	    // Give us some goal stats for this period, five at a time (because
	    // 10 metrics maximum).
	    $goals = $connector->getGoals($asset->attr('viewid'));
	    $goals_reports = Array();
	    foreach (array_chunk(array_keys($goals), 5) as $goalids) {
	        $goal_metrics = Array();
	        foreach ($goalids as $goalid) {
	            $goal_metrics[] = 'ga:goal'.$goalid.'Starts';
	            $goal_metrics[] = 'ga:goal'.$goalid.'Completions';
	        }
	        $goals_report = $asset->generateReport(
                Array(),
                $goal_metrics,
                $start_date->format('Y-m-d'),
                $end_date->format('Y-m-d')
	        );
	        $goals_reports = array_merge($goals_reports, $goals_report[0]);
	    }//end foreach

	    // But we still need to calculate our own rate, because Starts only
	    // has meaning for a funnel. Anything that doesn't have a funnel will
	    // have the same number of starts as completions, so reset that to
	    // the number of sessions.
	    foreach ($goals as $goal) {
	        if (($goal['type'] !== 'URL_DESTINATION') || ($goal['urlDestinationDetails']['firstStepRequired'] === FALSE)) {
	            $report['ga:goalStartsAll'] -= $goals_reports['ga:goal'.$goal['id'].'Starts'];
	            $report['ga:goalStartsAll'] += $report['ga:sessions'];

	            $goals_reports['ga:goal'.$goal['id'].'Starts'] = NULL;
	            if ($report['ga:sessions'] === 0) {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = NULL;
	            } else {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = $goals_reports['ga:goal'.$goal['id'].'Completions'] / $report['ga:sessions'];
	            }
	        } else {
	            if ($goals_reports['ga:goal'.$goal['id'].'Starts'] === 0) {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = NULL;
	            } else {
	                $goals_reports['sq:goal'.$goal['id'].'Rate'] = $goals_reports['ga:goal'.$goal['id'].'Completions'] / $goals_reports['ga:goal'.$goal['id'].'Starts'];
	            }
	        }
	    }//end foreach

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $day_count
	        );
	    $o->closeField();

	    // Start the section on goal stats.
	    $o->sectionNote(translate('For <strong>"URL (Funnel)" goals</strong>, only sessions that view the first page in the funnel are considered "starts".').
	        '<br/>'.
	        translate('For other goal types, all sessions are considered "starts" for the purposes of the Completion Rate presented here.'));
	    $o->openRaw();
	    ?>
	    <table class="sq-backend-table">
	    <col width="30%" />
	    <col width="25%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <thead>
	        <tr>
	            <th><?php echo translate('Goal Name') ?></th>
	            <th><?php echo translate('Goal Type') ?></th>
	            <th style="text-align: right"><?php echo translate('Completion Rate') ?></th>
	            <th style="text-align: right"><?php echo translate('Completions') ?></th>
	            <th style="text-align: right"><?php echo translate('Starts') ?></th>
	        </tr>
	    <tbody><?php
	        if (count($goals) === 0) {
	            ?><tr>
	            <td colspan="5"><?php echo translate('No goals have been created for this view'); ?></td>
	            </tr><?php
	        } else {
                ?><tr>
                    <th><?php echo translate('All goals') ?></th>
                    <td></td>
                    <td style="text-align: right"><strong><?php
                        if ($report['ga:goalStartsAll'] > 0) {
                            echo number_format($report['ga:goalCompletionsAll'] / $report['ga:goalStartsAll'] * 100.0, 1).'%';
                        } else {
                            echo translate('N/A');
                        }//end if
                    ?></strong></td>
                    <td style="text-align: right"><strong><?php echo number_format($report['ga:goalCompletionsAll'], 0); ?></strong></td>
                    <td style="text-align: right"><strong><?php echo number_format($report['ga:goalStartsAll'], 0); ?></strong></td>
                </tr><?php
                foreach ($goals as $goal) {
                ?><tr>
                    <th><?php echo $goal['id'].'. '.$goal['name'] ?></th>
                    <td><?php
                        switch ($goal['type']) {
                            case 'URL_DESTINATION':
                                if ($goal['urlDestinationDetails']['firstStepRequired'] === TRUE) {
                                    echo translate('URL (Funnel)');
                                } else {
                                    echo translate('URL (Single Page)');
                                }
                            break;
                            case 'EVENT':
                                echo translate('Event Tracking');
                            break;
                            case 'VISIT_NUM_PAGES':
                                echo translate('Page Views');
                            break;
                            case 'VISIT_TIME_ON_SITE':
                                echo translate('Session Duration');
                            break;
                            default:
                                echo translate('Unknown');
                            break;
                        }
                    ?></td>
                    <td style="text-align: right"><strong><?php
                        if ($goals_reports['sq:goal'.$goal['id'].'Rate'] !== NULL) {
                            echo number_format($goals_reports['sq:goal'.$goal['id'].'Rate'] * 100.0, 1).'%';
                        } else {
                            echo translate('N/A');
                        }//end if
                    ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($goals_reports['ga:goal'.$goal['id'].'Completions'], 0); ?></td>
                    <td style="text-align: right"><?php echo number_format(array_get_index($goals_reports, 'ga:goal'.$goal['id'].'Starts', $report['ga:sessions']), 0); ?></td>
                </tr><?php
                }//end foreach
            }//end if
	        ?></tbody>
	    </table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintGoalsReport()


	/**
	 * Paint the Content Experiments report.
	 *
	 */
	public function paintExperimentsReport($asset, &$o, $prefix)
	{
	    $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'google_analytics_connector', FALSE, NULL, 'minor');
	    $connector      = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['majorid']);

	    // Start the section on experiment management API info.
	    $experiments = $connector->getExperiments($asset->attr('viewid'));
	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="30%" />
	    <col width="15%" />
	    <col width="23%" />
	    <col width="8%" />
	    <col width="8%" />
	    <col width="8%" />
	    <col width="8%" />
	    <thead>
	        <tr>
	            <th><?php echo translate('Experiment name and description') ?></th>
	            <th><?php echo translate('Status') ?></th>
	            <th><?php echo translate('Variations') ?></th>
	            <th><?php echo translate('Traffic Coverage') ?></th>
	            <th><?php echo translate('Variation Weighting') ?></th>
	            <th><?php echo translate('Minimum Length') ?></th>
	            <th><?php echo translate('Winner Confidence') ?></th>
	        </tr>
	    <tbody><?php
	        if (count($experiments) === 0) {
	            ?><tr>
	            <td colspan="7"><?php echo translate('No content experiments available for this view'); ?></td>
	            </tr><?php
	        } else {
                foreach ($experiments as $experiment) {
                ?><tr>
                    <th style="font-weight: normal"><strong><?php echo $experiment['name'] ?></strong><br/>
                    Target metric: <strong><?php echo $experiment['objectiveMetric'] ?></strong><br/>
                    <?php echo ellipsisize($experiment['description'], 150); ?></th>
                    <td><?php
                        switch ($experiment['status']) {
                            case 'DRAFT':
                                echo translate('Draft');
                            break;
                            case 'READY_TO_RUN':
                                echo translate('Ready to run');
                            break;
                            case 'RUNNING':
                                echo translate('Running');
                            break;
                            case 'ENDED':
                                $end_time = strtotime($experiment['endTime']);
                                if (strtotime('+2 days', $end_time) > time()) {
                                    echo translate('Just ended');
                                } else {
                                    echo translate('Ended %s', date('jS M Y', $end_time));
                                }

                                echo '<br/>';
                                switch ($experiment['reasonExperimentEnded']) {
                                    case 'WINNER_FOUND':
                                        foreach (array_get_index($experiment, 'variations', Array()) as $variation) {
                                            if ($variation['won']) {
                                                echo translate('<strong>%s</strong> won', $variation['name']);
                                                break;
                                            }
                                        }
                                    break;
                                    case 'EXPERIMENT_EXPIRED':
                                        echo translate('Expired before winner found');
                                    break;
                                    case 'ENDED_WITH_NO_WINNER':
                                        echo translate('Ended without a winner');
                                    break;
                                    case 'STOPPED_BY_USER':
                                        echo translate('Stopped by user');
                                    break;
                                    default:
                                        // Do nothing.
                                    break;
                                }
                            break;
                            default:
                                echo translate('Unknown');
                            break;
                        }
                    ?></td>
                    <td><?php
                    $variation_list = Array();
                    foreach (array_get_index($experiment, 'variations', Array()) as $variation) {
                        $variation_str = $variation['name'];
                        if (array_get_index($variation, 'inactive', FALSE) === TRUE) {
                            $variation_str .= ' ('.translate('Inactive').')';
                        }
                        if (array_key_exists('weight', $variation) === TRUE) {
                            $variation_str .= ' ('.number_format($variation['weight'] * 100.0, 0).'%)';
                        }
                        $variation_list[] = $variation_str;
                    }
                    echo '<ol><li>'.implode('</li><li>', $variation_list).'</li></ol>';
                    ?></td>
                    <td><?php echo translate('%s%%', $experiment['trafficCoverage'] * 100.0) ?></td>
                    <td><?php
                    if ($experiment['equalWeighting'] === TRUE) {
                        echo translate('Equal');
                    } else {
                        echo translate('Dynamic');
                    } ?></td>
                    <td><?php echo translate('%s days', $experiment['minimumExperimentLengthInDays']) ?></td>
                    <td><?php echo translate('%s%%', number_format($experiment['winnerConfidenceLevel'] * 100.0, 1)) ?></td>
                </tr><?php
                }//end foreach
            }//end if
	        ?></tbody>
	    </table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintExperimentsReport()

	/**
	 * Paint the geographic reports.
	 *
	 */
	public function paintGeoCountryReport($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:country',
	            'ga:region',
	            'ga:city',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d'),
	        Array('-ga:sessions')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $city_report    = Array();

	    $i = 0;
	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:country'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $country = $row['ga:country'];
	            $city    = $row['ga:city'];

	            if ($city === self::NOT_SET) {
	                $city = $row['ga:region'];
	            } else if ($row['ga:region'] !== self::NOT_SET) {
	                $city .= ', '.$row['ga:region'];
	            }

	            if ($city !== self::NOT_SET) {
	                $i++;
	                $city_report[$country] = array_get_index($city_report, $country, Array());
                    $city_report[$country][$city] = $row;
                    $city_report[$country][$city]['sq:cityRegion'] = $city;
                    $city_report[$country][$city]['sq:rank'] = $i;
                }

	            if (array_key_exists($country, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (is_numeric($value) === TRUE) {
	                        $report[$country][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$country] = $row;
	            }
	        }
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    foreach ($city_report as $country => &$city_rows) {
            usort($city_rows, function($a, $b) {
                return $a['sq:rank'] - $b['sq:rank'];
            });
            $city_rows = array_slice($city_rows, 0, 5);
	    }

	    $max_results = 20;
	    $other_report = array_reduce(
	        array_slice($report, $max_results),
	        function($carry, $row) {
	            if (empty($carry) === FALSE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (is_numeric($value) === TRUE) {
	                        $carry[$fieldid] += $value;
	                    }
	                }
	            } else {
	                $carry = $row;
	            }

	            return $carry;
	        },
	        Array()
	    );

	    $other_report['ga:country'] = translate('Other countries (%s)', count($report) - $max_results);
	    $other_report['sq:other']   = TRUE;
	    $report = array_slice($report, 0, $max_results);
	    $report[] = $other_report;

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provide location information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Country') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php
                $country = $row['ga:country'];
                if (array_key_exists('sq:other', $row) === FALSE) {
                    echo $rank.'. '.$row['ga:country'];
                } else {
                    echo $row['ga:country'];
                } ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                if (array_key_exists('sq:other', $row) === FALSE) {
                    foreach ($city_report[$country] as $city_row) {
                    ?><tr>
                        <td><?php
                        echo '<span style="float: left; padding-left: 3ex">';
                        echo $city_row['sq:cityRegion'];
                        echo '</span><span style="float: right"><em>(#'.$city_row['sq:rank'].')</em></span>';
                        ?></td>
                        <td style="text-align: right"><strong><?php echo number_format($city_row['ga:sessions'], 0); ?></strong></td>
                        <td style="text-align: right"><?php echo number_format($city_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                        <td style="text-align: right"><?php echo number_format($city_row['ga:users'], 0); ?></td>
                        <td style="text-align: right"><?php
                        if ($city_row['ga:sessions'] === 0) {
                            echo translate('N/A');
                        } else {
                            echo number_format($city_row['ga:pageviews'] / $city_row['ga:sessions'], 1);
                        }
                        ?></td>
                        <td style="text-align: right"><?php
                        if ($city_row['ga:sessions'] === 0) {
                            echo translate('N/A');
                        } else {
                            $time = $city_row['ga:sessionDuration'] / $city_row['ga:sessions'];
                            if ($time >= 60) {
                                echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                            } else {
                                echo sprintf(translate('%s sec', '%s'), floor($time));
                            }
                        }
                        ?></td>
                    </tr><?php
                    }//end if city rows
                }//end if not other
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintGeographicReport()


	/**
	 * Paint the geographic reports.
	 *
	 */
	public function paintGeoCityReport($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    $report = $asset->generateReport(
	        Array(
	            'ga:country',
	            'ga:region',
	            'ga:city',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d'),
	        Array('-ga:sessions')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    foreach ($report['rows'] as $rowid => &$row) {
	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:city'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        }
	    }//end foreach

	    $report = array_slice($report, 0, 50);

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provide a city',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="20%" />
	    <col width="20%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('City Name') ?></th>
	            <th><?php echo translate('Country') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report['rows'] as $row) {
	            $rank++;
            ?><tr>
                <th><?php echo $rank.'. '.$row['ga:city']; ?></th>
                <td><?php echo $row['ga:country']; ?></strong></td>
                <td style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></td>
                <td style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                <td style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></td>
                <td style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></td>
                <td style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></td>
            </tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintGeoCityReport()


//--        TECHNOLOGY REPORTS        --//




	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesOSesReportDesktop($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'desktop') {
	            continue;
	        }

	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:operatingSystem'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:operatingSystem'];
	            $version = $row['ga:operatingSystemVersion'];

	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if

	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided operating system information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Operating System') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
	            $translated_row = $this->_translateDesktopOS(
                    $row['ga:operatingSystem'],
                    self::NOT_SET
                );
            ?><tr>
                <th><?php
                $os = $row['ga:operatingSystem'];
                echo $rank.'. '.$translated_row['ga:operatingSystem'];
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                $translated_row = $this->_translateDesktopOS(
                    $version_row['ga:operatingSystem'],
                    $version_row['ga:operatingSystemVersion']
                );
                ?><tr>
                    <td><?php
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $translated_row['ga:operatingSystemVersion'].'</span>';
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintTechnologiesOSesReportDesktop()




	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesOSesReportTablet($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'tablet') {
	            continue;
	        }

	        $row = array_merge(
	            $row,
	            $this->_translateMobileOS(
	                $row['ga:operatingSystem'],
	                $row['ga:operatingSystemVersion']
	            )
	        );

	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:operatingSystem'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:operatingSystem'];
	            $version = $row['ga:operatingSystemVersion'];

	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if

	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided operating system information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Operating System') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php
                $os = $row['ga:operatingSystem'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:operatingSystemVersion'].'</span>';
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintTechnologiesOSesReportTablet()


	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesOSesReportMobile($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'mobile') {
	            continue;
	        }

	        $row = array_merge(
	            $row,
	            $this->_translateMobileOS(
	                $row['ga:operatingSystem'],
	                $row['ga:operatingSystemVersion']
	            )
	        );

	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:operatingSystem'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:operatingSystem'];
	            $version = $row['ga:operatingSystemVersion'];

	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if

	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:operatingSystemVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided operating system information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Operating System') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php
                $os = $row['ga:operatingSystem'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:operatingSystemVersion'].'</span>';
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintTechnologiesOSesReportMobile()



	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesBrowsersReportDesktop($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'desktop') {
	            continue;
	        }

	        $row = array_merge(
	            $row,
	            $this->_translateDesktopBrowser(
	                $row['ga:browser'],
	                $row['ga:browserVersion']
	            )
	        );

	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:browser'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:browser'];
	            $version = $row['ga:browserVersion'];

	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if ((($fieldid !== 'ga:operatingSystemVersion') && ($fieldid !== 'ga:browserVersion')) && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if

	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if ((($fieldid !== 'ga:operatingSystemVersion') && ($fieldid !== 'ga:browserVersion')) && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided browser information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php
                $os = $row['ga:browser'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:browserVersion'].'</span>';
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintTechnologiesOSesReportMobile()
	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesBrowsersReportTablet($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'tablet') {
	            continue;
	        }

	        $row = array_merge(
	            $row,
	            $this->_translateMobileBrowser(
	                $row['ga:browser'],
	                $row['ga:browserVersion']
	            )
	        );

	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:browser'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:browser'];
	            $version = $row['ga:browserVersion'];

	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if

	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided browser information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php
                $os = $row['ga:browser'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:browserVersion'].'</span>';
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintTechnologiesOSesReportMobile()

	/**
	 * Paint the technologies OS report.
	 *
	 */
	public function paintTechnologiesBrowsersReportMobile($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:operatingSystem',
	            'ga:operatingSystemVersion',
	            'ga:browser',
	            'ga:browserVersion',
            ),
	        Google_Analytics_View::SITE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();
	    $version_report = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'mobile') {
	            continue;
	        }

	        $row = array_merge(
	            $row,
	            $this->_translateMobileBrowser(
	                $row['ga:browser'],
	                $row['ga:browserVersion']
	            )
	        );

	        $total_sessions += $row['ga:sessions'];
	        if ($row['ga:browser'] === self::NOT_SET) {
	            $crop_sessions += $row['ga:sessions'];
                unset($report[$rowid]);
	        } else {
	            $os      = $row['ga:browser'];
	            $version = $row['ga:browserVersion'];

	            if ($os !== self::NOT_SET) {
	                $version_report[$os] = array_get_index($version_report, $os, Array());
                    if (array_key_exists($version, $version_report[$os]) === TRUE) {
                        foreach ($row as $fieldid => &$value) {
                            if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
                                $version_report[$os][$version][$fieldid] += $value;
                            }
                        }
                    } else {
                        $version_report[$os][$version] = $row;
                    }
                }//end if

	            if (array_key_exists($os, $report) === TRUE) {
	                foreach ($row as $fieldid => &$value) {
	                    if (($fieldid !== 'ga:browserVersion') && (is_numeric($value) === TRUE)) {
	                        $report[$os][$fieldid] += $value;
	                    }
	                }
	            } else {
	                $report[$os] = $row;
	            }
	        }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:sessions'] - $a['ga:sessions'];
	    });

	    foreach ($version_report as $os => &$version_rows) {
            usort($version_rows, function($a, $b) {
                return $b['ga:sessions'] - $a['ga:sessions'];
            });
	    }

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Sessions'));
            echo translate(
                '%s / %s sessions analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all session details provided browser information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="40%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="15%" />
	    <col width="15%" />
	    <col width="15%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Sessions') ?></td>
	            <th><?php echo translate('Unique Users') ?></td>
	            <th><?php echo translate('Page Views / Session') ?></td>
	            <th><?php echo translate('Session Duration') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <th><?php
                $os = $row['ga:browser'];
                echo $rank.'. '.$os;
                ?></th>
                <th style="text-align: right"><strong><?php echo number_format($row['ga:sessions'], 0); ?></strong></th>
                <th style="text-align: right"><?php echo number_format($row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <th style="text-align: right"><?php echo number_format($row['ga:users'], 0); ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    echo number_format($row['ga:pageviews'] / $row['ga:sessions'], 1);
                }
                ?></th>
                <th style="text-align: right"><?php
                if ($row['ga:sessions'] === 0) {
                    echo translate('N/A');
                } else {
                    $time = $row['ga:sessionDuration'] / $row['ga:sessions'];
                    if ($time >= 60) {
                        echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                    } else {
                        echo sprintf(translate('%s sec', '%s'), floor($time));
                    }
                }
                ?></th><?php
                foreach ($version_report[$os] as $version_row) {
                ?><tr>
                    <td><?php
                    echo '<span style="float: left; padding-left: 3ex">';
                    echo $version_row['ga:browserVersion'].'</span>';
                    ?></td>
                    <td style="text-align: right"><strong><?php echo number_format($version_row['ga:sessions'], 0); ?></strong></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:sessions'] / $total_sessions * 100.0, 2).'%'; ?></td>
                    <td style="text-align: right"><?php echo number_format($version_row['ga:users'], 0); ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        echo number_format($version_row['ga:pageviews'] / $version_row['ga:sessions'], 1);
                    }
                    ?></td>
                    <td style="text-align: right"><?php
                    if ($version_row['ga:sessions'] === 0) {
                        echo translate('N/A');
                    } else {
                        $time = $version_row['ga:sessionDuration'] / $version_row['ga:sessions'];
                        if ($time >= 60) {
                            echo sprintf(translate('%s min %s sec', '%s', '%s'), (int)($time / 60), str_pad(floor($time % 60), 2, '0', STR_PAD_LEFT));
                        } else {
                            echo sprintf(translate('%s sec', '%s'), floor($time));
                        }
                    }
                    ?></td>
                </tr><?php
                }//end if city rows
            ?></tr><?php
            }
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintTechnologiesOSesReportMobile()


	private function _translateDesktopOS($os, $version) {
	    switch ($os) {
    	    case 'Windows':
    	        $version = 'Windows '.$version;
	        break;

		    case 'Macintosh':
		        $os = 'OS X (Mac)';

		        // Remove the architecture.
		        if (strpos($version, ' ') === FALSE) {
		            // We have arch but nothing else!
		            $version = self::NOT_SET;
		        } else {
		            list($arch, $version) = explode(' ', $version);

                    $version_list = Array(
                        '10.10' => 'Yosemite',
                        '10.9'  => 'Mavericks',
                        '10.8'  => 'Mountain Lion',
                        '10.7'  => 'Lion',
                        '10.6'  => 'Snow Leopard',
                        '10.5'  => 'Leopard',
                        '10.4'  => 'Tiger',
                        '10.3'  => 'Panther',
                    );

                    if (array_key_exists($version, $version_list) === TRUE) {
                        $version = $version_list[$version].' ('.$version.')';
                    }
                }//end if
	        break;

		    case 'Linux':
		        if (($version === 'x86_64') || ($version === 'amd64')) {
		            $version = translate('64-bit (x86-64)');
		        } else if ($version === 'ia64') {
		            $version = translate('64-bit (Itanium)');
		        } else if (preg_match('/^i[3-6]86$/', $version) > 0) {
		            $version = translate('32-bit');
		        } else {
		            $version = self::NOT_SET;
		        }
	        break;
	    }//end switch

	    return Array(
	        'ga:operatingSystem'        => $os,
	        'ga:operatingSystemVersion' => $version,
	    );

	}//end translateDesktopOSName()


	private function _translateMobileOS($os, $version) {
	    switch ($os) {
		    case 'iOS':
		        if ($version === self::NOT_SET) {
		            $os = translate('iOS (unknown version)');
		        } else {
		            $version_parts = explode('.', $version);
		            $os = 'iOS '.$version_parts[0];
		        }
	        break;

		    case 'Android':
		        $version_list = Array(
		            '4\.4'     => 'KitKat',
		            '4\.[123]' => 'Jelly Bean',
		            '4\.0'     => 'Ice Cream Sandwich',
		            '3\.[012]' => 'Honeycomb',
		            '2\.3'     => 'Gingerbread',
		            '2\.2'     => 'Froyo',
		            '2\.[01]'  => 'Eclair',
		            '1\.6'     => 'Donut',
		            '1\.5'     => 'Cupcake',
		        );

		        if ($version === self::NOT_SET) {
		            $os = translate('Android (unknown version)');
		        } else {
                    $os = translate('Android 1.1 or lower');
                    foreach ($version_list as $regex => $version_name) {
                        if (preg_match('/^'.$regex.'(\.[\d.]+)?$/', $version) > 0) {
                            $os = 'Android '.$version_name;
                            break;
                        }
                    }
		        }
	        break;
	    }//end switch

	    return Array(
	        'ga:operatingSystem'        => $os,
	        'ga:operatingSystemVersion' => $version,
	    );

	}//end translateDesktopOSName()


	private function _translateDesktopBrowser($browser, $version) {
	    switch ($browser) {
	        case 'Chrome':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';

	            $version_list = Array(
	                '32.0' => translate('2014 (versions 32+)'),
	                '28.0' => translate('2013, Blink (versions 28-31)'),
	                '24.0' => translate('2013, WebKit (versions 24-27)'),
	                '17.0' => translate('2012 (versions 17-23)'),
	                '9.0'  => translate('2011 (versions 9-16)'),
	                '0.0'  => translate('2010 or earlier (versions 0.x-8)'),
	            );

	            foreach ($version_list as $version_minimum => $version_str) {
	                if (version_compare($version, $version_minimum, '>=') === TRUE) {
	                    $version = $version_str;
	                    break;
	                }
	            }
	        break;

	        case 'Firefox':
                $version_parts = explode('.', $version);
                if ((int) $version_parts[0] >= 4) {
                    $esr_version = floor(($version_parts[0] - 3) / 7) * 7 + 3;
                    if ($esr_version < 10) {
                        // Pre-ESR. Group rapid releases of 4.x+ together.
                        $esr_version      = 4;
                        $esr_last_version = 9;
                    } else {
                        // By rule, ESR versions currently span 7 rapid relases.
                        $esr_last_version = $esr_version + 6;
                    }
                    $version = $esr_version.'-'.$esr_last_version;
	            } else {
                    $version = $version_parts[0].'.'.$version_parts[1];
                }
	        break;

	        case 'Safari':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;

	        case 'Opera':
	            $version_parts = explode('.', $version);

	            if ($version_parts[0] >= 15) {
	                // Blink-based Opera.
                    $version_list = Array(
                        '19.0'  => translate('Blink, 2014 (versions 19+)'),
                        '15.0'  => translate('Blink, 2013 (versions 15-18)'),
                    );

                    foreach ($version_list as $version_minimum => $version_str) {
                        if (version_compare($version, $version_minimum, '>=') === TRUE) {
                            $version = $version_str;
                            break;
                        }
                    }
	            } else {
	                // Classic Opera. Format it as, eg. '12.5x'
	                $version = translate('Presto (up to version 12)');
	                //$version = $version_parts[0].'.'.floor($version_parts[1] / 10).'x';
	            }
	        break;

	        case 'Internet Explorer':
	            if ($version === '999.1') {
	                // Certain anti-viruses set IE's version to a dodgy value
	                // to stop people gleaning security issues from their
	                // version number. Not helpful.
	                $version = self::NOT_SET;
	            }
	    }

	    return Array(
	        'ga:browser'        => $browser,
	        'ga:browserVersion' => $version,
	    );

	    return Array(
	        'ga:browser'        => $browser,
	        'ga:browserVersion' => $version,
	    );

	}//end translateDesktopOSName()


	private function _translateMobileBrowser($browser, $version) {
	    switch ($browser) {
	        case 'Chrome':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;

	        case 'Firefox':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;

	        case 'Safari':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.x';
	        break;

	        case 'Opera Mini':
	            $version_parts = explode('.', $version);
	            $version = $version_parts[0].'.'.$version_parts[1];
	        break;
	    }

	    return Array(
	        'ga:browser'        => $browser,
	        'ga:browserVersion' => $version,
	    );

	}//end translateDesktopOSName()


//--        CONTENT REPORTS        --//


	/**
	 * Paint the Content screen's "Most viewed content, non-mobile" report.
	 *
	 */
	public function paintContentMostViewedReportDesktop($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:hostname',
	            'ga:pagePath'
            ),
	        Google_Analytics_View::PAGE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] === 'mobile') {
	            continue;
	        }

	        $total_sessions += $row['ga:pageviews'];
	        $row['sq:domainPath'] = $row['ga:hostname'].$row['ga:pagePath'];
	        $path = $row['sq:domainPath'];
            if (array_key_exists($path, $report) === TRUE) {
                foreach ($row as $fieldid => &$value) {
                    if (is_numeric($value) === TRUE) {
                        $report[$path][$fieldid] += $value;
                    }
                }
            } else {
                $report[$path] = $row;
            }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:pageviews'] - $a['ga:pageviews'];
	    });

	    $report = array_slice($report, 0, 50);

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Page Views'));
            echo translate(
                '%s / %s page views analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all page view hits provided page path information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="55%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="10%" />
	    <col width="10%" />
	    <col width="10%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Page Views') ?></td>
	            <th><?php echo translate('Landings') ?></td>
	            <th><?php echo translate('Exits') ?></td>
	            <th><?php echo translate('Bounces') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <td><?php
                echo $rank.'. '.$row['sq:domainPath'];
                ?></td>
                <td style="text-align: right"><strong><?php echo number_format($row['ga:pageviews'], 0); ?></strong></th>
                <td style="text-align: right"><?php echo number_format($row['ga:pageviews'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:entrances'], 0); ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:exits'], 0); ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:bounces'], 0); ?></th>
                </tr><?php
            }//end foreach
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintContentMostViewedReportDesktop()




	/**
	 * Paint the Content screen's "Most viewed content on mobile" report.
	 *
	 */
	public function paintContentMostViewedReportMobile($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report  = $asset->generateReport(
	        Array(
	            'ga:deviceCategory',
	            'ga:hostname',
	            'ga:pagePath'
            ),
	        Google_Analytics_View::PAGE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:deviceCategory'] !== 'mobile') {
	            continue;
	        }

	        $total_sessions += $row['ga:pageviews'];
	        $row['sq:domainPath'] = $row['ga:hostname'].$row['ga:pagePath'];
	        $path = $row['sq:domainPath'];
            if (array_key_exists($path, $report) === TRUE) {
                foreach ($row as $fieldid => &$value) {
                    if (is_numeric($value) === TRUE) {
                        $report[$path][$fieldid] += $value;
                    }
                }
            } else {
                $report[$path] = $row;
            }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:pageviews'] - $a['ga:pageviews'];
	    });

	    $report = array_slice($report, 0, 50);

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Page Views'));
            echo translate(
                '%s / %s page views analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all page view hits provided page path information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="55%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="10%" />
	    <col width="10%" />
	    <col width="10%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Browser') ?></th>
	            <th colspan="2"><?php echo translate('Page Views') ?></td>
	            <th><?php echo translate('Landings') ?></td>
	            <th><?php echo translate('Exits') ?></td>
	            <th><?php echo translate('Bounces') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <td><?php
                echo $rank.'. '.$row['sq:domainPath'];
                ?></td>
                <td style="text-align: right"><strong><?php echo number_format($row['ga:pageviews'], 0); ?></strong></th>
                <td style="text-align: right"><?php echo number_format($row['ga:pageviews'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:entrances'], 0); ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:exits'], 0); ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:bounces'], 0); ?></th>
                </tr><?php
            }//end foreach
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintContentMostViewedReportDesktop()




	/**
	 * Paint the Content screen's "Most viewed content by returning users" report.
	 *
	 */
	public function paintContentMostViewedReportReturningUsers($asset, &$o, $prefix)
	{
	    $period = 'P1M';

	    $end_date    = new DateTime('today 00:00:00');
	    $start_date  = clone $end_date;
	    $start_date->sub(new DateInterval($period));
	    $date_diff   = $start_date->diff($end_date);
	    $start_date->add(new DateInterval('P1D'));

	    // Grab all the reports.
	    $raw_report = $asset->generateReport(
	        Array(
	            'ga:sessionCount',
	            'ga:hostname',
	            'ga:pagePath'
            ),
	        Google_Analytics_View::PAGE_METRICS,
	        $start_date->format('Y-m-d'),
	        $end_date->format('Y-m-d')
	    );

	    $total_sessions = 0;
	    $crop_sessions  = 0;
	    $report         = Array();

	    foreach ($raw_report['rows'] as $rowid => &$row) {
	        if ($row['ga:sessionCount'] === '1') {
	            continue;
	        }

	        $total_sessions += $row['ga:pageviews'];
	        $row['sq:domainPath'] = $row['ga:hostname'].$row['ga:pagePath'];
	        $path = $row['sq:domainPath'];
            if (array_key_exists($path, $report) === TRUE) {
                foreach ($row as $fieldid => &$value) {
                    if (is_numeric($value) === TRUE) {
                        $report[$path][$fieldid] += $value;
                    }
                }
            } else {
                $report[$path] = $row;
            }//end if
	    }//end foreach

	    usort($report, function($a, $b) {
	        return $b['ga:pageviews'] - $a['ga:pageviews'];
	    });

	    $report = array_slice($report, 0, 50);

	    $o->openField(translate('Reporting Period'));
	        echo sprintf(
	            translate('%s until %s (%s days)', '%s', '%s', '%s'),
	            '<strong>'.$start_date->format('jS M Y').'</strong>',
	            '<strong>'.$end_date->format('jS M Y').'</strong>',
	            $date_diff->format('%a')
	        );
	    $o->closeField();

	    $o->openField(translate('Total Page Views'));
            echo translate(
                '%s / %s page views analysed',
                number_format(($total_sessions - $crop_sessions), 0),
                number_format($total_sessions, 0)
            );

            if ($crop_sessions > 0) {
                $o->note(translate(
                    'Only %s%% of all page view hits provided page path information',
                    number_format(((1 - ($crop_sessions / $total_sessions)) * 100.0), 2)
                ));
            }
        $o->closeField();

	    $o->openRaw();
	    ?><table class="sq-backend-table">
	    <col width="55%" />
	    <col width="8.5%" />
	    <col width="6.5%" />
	    <col width="10%" />
	    <col width="10%" />
	    <col width="10%" />
	    <tbody>
	        <tr>
	            <th><?php echo translate('Page Path') ?></th>
	            <th colspan="2"><?php echo translate('Page Views') ?></td>
	            <th><?php echo translate('Landings') ?></td>
	            <th><?php echo translate('Exits') ?></td>
	            <th><?php echo translate('Bounces') ?></td>
	        </tr><?php
	        $rank = 0;
	        foreach ($report as $row) {
	            $rank++;
            ?><tr>
                <td><?php
                echo $rank.'. '.$row['sq:domainPath'];
                ?></td>
                <td style="text-align: right"><strong><?php echo number_format($row['ga:pageviews'], 0); ?></strong></th>
                <td style="text-align: right"><?php echo number_format($row['ga:pageviews'] / $total_sessions * 100.0, 2).'%'; ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:entrances'], 0); ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:exits'], 0); ?></th>
                <td style="text-align: right"><?php echo number_format($row['ga:bounces'], 0); ?></th>
                </tr><?php
            }//end foreach
            ?></tbody></table><?php
	    $o->closeRaw();

	    return FALSE;

	}//end paintContentMostViewedReportReturningUsers()


	/**
	 * Paints an interface for selecting a Google Analytics connector.
	 *
	 * @param Google_Analytics_View $asset   Asset to display
	 * @param Backend_Outputter     $o       Relevant backend outputter
	 * @param string                $prefix  Prefix for form elements
	 *
	 * @return boolean
	 */
	public function paintSelectConnector(Google_Analytics_View $asset, Backend_Outputter $o, $prefix)
	{
	    $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'google_analytics_connector', TRUE, NULL, 'minor');

	    $wa = $asset->writeAccess('links');

	    if ($wa === FALSE) {
	        if (empty($connector_link) === TRUE) {
	            echo translate('No connector set up yet');
	        } else {
	            echo get_asset_tag_line($connector_link['majorid']);
	        }

	        return FALSE;
	    } else {
	        $assetid = 0;
	        if (empty($connector_link) === FALSE) {
	            $assetid = $connector_link['majorid'];
	        }

	        asset_finder(
                $prefix.'_connector',
                $assetid,
                Array(
                    'google_analytics_connector' => 'I'
                )
            );
        }

        return $wa;

	}//end paintSelectView()


	/**
	 * Processes the Google Analytics connector interface.
	 *
	 * Changing the connection will clear out the available data.
	 *
	 * @param Google_Analytics_View $asset   Asset to display
	 * @param Backend_Outputter     $o       Relevant backend outputter
	 * @param string                $prefix  Prefix for form elements
	 *
	 * @return boolean
	 */
	public function processSelectConnector(Google_Analytics_View $asset, Backend_Outputter $o, $prefix)
	{
	    $wa = $asset->writeAccess('links');

	    if ($wa === TRUE) {
            $connector_post    = array_get_index($_POST, $prefix.'_connector', Array());
            $connector_assetid = array_get_index($connector_post, 'assetid');

            if (empty($connector_assetid) === FALSE) {
                $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'google_analytics_connector', TRUE, NULL, 'minor');
                if (empty($connector_link) === FALSE) {
                    if ($connector_link['majorid'] !== $connector_assetid) {
                        $GLOBALS['SQ_SYSTEM']->am->moveLink($connector_link['linkid'], $connector_assetid, SQ_LINK_NOTICE, -1);
                    }
                } else {
                    $connector = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_assetid);
                    $GLOBALS['SQ_SYSTEM']->am->createAssetLink($connector, $asset, SQ_LINK_NOTICE);
                }//end if (connector link exists)
            }//end if (connector ID posted)
        }//end if (write access)

        return $wa;

	}//end processSelectConnector()


	/**
	 * Paints a list of views and a method to create a view.
	 *
	 * @param Google_Analytics_View $asset  The asset being painted.
	 * @param Backend_Outputter     $o      The relevant outputter.
	 * @param string                $prefix The prefix for form elements.
	 *
	 * @return boolean
	 */
	public function paintSelectView(Google_Analytics_View $asset, Backend_Outputter $o, $prefix)
	{
	    $wa = $asset->writeAccess('attributes');

	    $connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'google_analytics_connector', TRUE, NULL, 'minor');
	    if (empty($connector_link) === TRUE) {
	        echo translate('You need to connect this view to a Google Analytics Connector before you can select a view.');
			return FALSE;
		}

	    $connector_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['majorid']);
	    $current_viewid  = $asset->attr('viewid');

		$access_token = $connector_asset->attr('global_access_token');
		if (empty($connector_link) === TRUE) {
	        echo translate('You need to authorise the account before you can see the available views.');
			return FALSE;
		}

		$properties   = $connector_asset->getProperties();
		$views        = $connector_asset->getViews();

		if ($wa === FALSE) {
		    if (array_key_exists($current_viewid, $views) === TRUE) {
		        echo sprintf(translate('%1$s [ga:%2$s]', '%1$s', '%2$s'), $views[$current_viewid]['name'], $current_viewid);
		    } else {
		        echo sprintf(translate('Unknown or inaccessible view [ga:%s]', '%s'), $current_viewid);
		    }
		} else {
            echo '<table class="sq-backend-table">';
            echo '<col width="5%" />';
            echo '<col width="50%" />';
            echo '<col width="45%" />';

            echo '<thead>';
            echo '<tr>';
            echo '<th></th>';

            echo '<th>'.translate('View Name').'</th>';
            echo '<th>'.translate('Property ID').'</th>';
            echo '</tr>';
            echo '</thead><tbody>';

            foreach ($views as $view) {
                $propertyid = $view['sq:propertyId'];
                echo '<tr>';
                echo '<td>';
                $selected = FALSE;
                if ($current_viewid === $view['id']) {
                    $selected = TRUE;
                }

                radio_button($prefix.'_select_view', $view['id'], $selected);
                echo '</td>';

                echo '<td><label for="'.$prefix.'_select_view_'.$view['id'].'"><strong>'.$view['name'].'</strong></label></td>';
                echo '<td>'.$properties[$propertyid]['name'].' ('.$propertyid.')</td>';
                echo '</tr>';
            }//end foreach

            echo '</tbody></table>';
        }//end if (write access)

		return $wa;

	}//end paintViewList()


	/**
	 * Processes a list of views and a method to create a view.
	 *
	 * @param Google_Analytics_View $asset  The asset being painted.
	 * @param Backend_Outputter     $o      The relevant outputter.
	 * @param string                $prefix The prefix for form elements.
	 *
	 * @return boolean
	 */
	public function processSelectView(Google_Analytics_View $asset, Backend_Outputter $o, $prefix)
	{
		$wa     = $asset->writeAccess('attributes');
		$viewid = array_get_index($_POST, $prefix.'_select_view');

		if (($viewid !== NULL) && ($wa === TRUE)) {
		    $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		    $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		    $GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes');
		    $asset->setAttrValue('viewid', $viewid);
		    $asset->saveAttributes();
		    $GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');

		    $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		    $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}

		return $wa;

	}//end processSelectView()


}//end class

?>
