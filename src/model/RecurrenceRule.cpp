#include "RecurrenceRule.h"

RecurrenceRule::RecurrenceRule(Frequency frequency,
                               int interval,
                               EndMode endMode,
                               const QDateTime& untilDate,
                               int maxOccurrences)
    : m_frequency(frequency),
      m_interval(interval < 1 ? 1 : interval),
      m_endMode(endMode),
      m_untilDate(untilDate),
      m_maxOccurrences(maxOccurrences < 1 ? 1 : maxOccurrences)
{
}

RecurrenceRule::Frequency RecurrenceRule::frequency() const
{
    return m_frequency;
}

void RecurrenceRule::setFrequency(Frequency frequency)
{
    m_frequency = frequency;
}

int RecurrenceRule::interval() const
{
    return m_interval;
}

void RecurrenceRule::setInterval(int interval)
{
    m_interval = interval < 1 ? 1 : interval;
}

RecurrenceRule::EndMode RecurrenceRule::endMode() const
{
    return m_endMode;
}

void RecurrenceRule::setEndMode(EndMode endMode)
{
    m_endMode = endMode;
}

QDateTime RecurrenceRule::untilDate() const
{
    return m_untilDate;
}

void RecurrenceRule::setUntilDate(const QDateTime& untilDate)
{
    m_untilDate = untilDate;
}

int RecurrenceRule::maxOccurrences() const
{
    return m_maxOccurrences;
}

void RecurrenceRule::setMaxOccurrences(int maxOccurrences)
{
    m_maxOccurrences = maxOccurrences < 1 ? 1 : maxOccurrences;
}

bool RecurrenceRule::isValid() const
{
    if (m_interval < 1) {
        return false;
    }

    if (m_endMode == EndMode::UntilDate && !m_untilDate.isValid()) {
        return false;
    }

    if (m_endMode == EndMode::AfterOccurrences && m_maxOccurrences < 1) {
        return false;
    }

    return true;
}

QDateTime RecurrenceRule::nextOccurrenceAfter(const QDateTime& firstOccurrence,
                                              const QDateTime& after) const
{
    if (!firstOccurrence.isValid() || !after.isValid() || !isValid()) {
        return QDateTime();
    }

    QDateTime occurrence = firstOccurrence;
    int occurrenceIndex = 1;

    const int maxSafetyIterations = 10000;
    int iterations = 0;

    while (iterations < maxSafetyIterations) {
        if (!occurrenceAllowed(occurrence, occurrenceIndex)) {
            return QDateTime();
        }

        if (occurrence > after) {
            return occurrence;
        }

        occurrence = addIntervalTo(occurrence);
        ++occurrenceIndex;
        ++iterations;
    }

    return QDateTime();
}

QString RecurrenceRule::toDisplayString() const
{
    QString text = QString("Every %1 %2")
            .arg(m_interval)
            .arg(frequencyToString(m_frequency));

    if (m_interval == 1) {
        text = QString("Every %1").arg(frequencyToString(m_frequency));
    }

    switch (m_endMode) {
    case EndMode::Never:
        text += " | Never ends";
        break;

    case EndMode::UntilDate:
        text += QString(" | Until %1").arg(m_untilDate.toString("yyyy-MM-dd HH:mm"));
        break;

    case EndMode::AfterOccurrences:
        text += QString(" | For %1 occurrence(s)").arg(m_maxOccurrences);
        break;
    }

    return text;
}

QDateTime RecurrenceRule::addIntervalTo(const QDateTime& dateTime) const
{
    switch (m_frequency) {
    case Frequency::Daily:
        return dateTime.addDays(m_interval);

    case Frequency::Weekly:
        return dateTime.addDays(7 * m_interval);

    case Frequency::Monthly:
        return dateTime.addMonths(m_interval);

    case Frequency::Yearly:
        return dateTime.addYears(m_interval);
    }

    return dateTime;
}

bool RecurrenceRule::occurrenceAllowed(const QDateTime& occurrenceDateTime,
                                       int occurrenceIndex) const
{
    switch (m_endMode) {
    case EndMode::Never:
        return true;

    case EndMode::UntilDate:
        return occurrenceDateTime <= m_untilDate;

    case EndMode::AfterOccurrences:
        return occurrenceIndex <= m_maxOccurrences;
    }

    return false;
}

QString RecurrenceRule::frequencyToString(Frequency frequency)
{
    switch (frequency) {
    case Frequency::Daily:
        return "day";

    case Frequency::Weekly:
        return "week";

    case Frequency::Monthly:
        return "month";

    case Frequency::Yearly:
        return "year";
    }

    return "period";
}