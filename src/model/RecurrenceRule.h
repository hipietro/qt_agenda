// Recurrence settings for repeated activities. Kept independent from GUI code.

#ifndef RECURRENCERULE_H
#define RECURRENCERULE_H

#include <QDateTime>
#include <QString>

class RecurrenceRule
{
public:
    enum class Frequency
    {
        Daily,
        Weekly,
        Monthly,
        Yearly
    };

    enum class EndMode
    {
        Never,
        UntilDate,
        AfterOccurrences
    };

    RecurrenceRule(Frequency frequency = Frequency::Weekly,
                   int interval = 1,
                   EndMode endMode = EndMode::AfterOccurrences,
                   const QDateTime& untilDate = QDateTime(),
                   int maxOccurrences = 1);

    Frequency frequency() const;
    void setFrequency(Frequency frequency);

    int interval() const;
    void setInterval(int interval);

    EndMode endMode() const;
    void setEndMode(EndMode endMode);

    QDateTime untilDate() const;
    void setUntilDate(const QDateTime& untilDate);

    int maxOccurrences() const;
    void setMaxOccurrences(int maxOccurrences);

    bool isValid() const;

    QDateTime nextOccurrenceAfter(const QDateTime& firstOccurrence,
                                  const QDateTime& after) const;

    QString toDisplayString() const;

private:
    QDateTime addIntervalTo(const QDateTime& dateTime) const;
    bool occurrenceAllowed(const QDateTime& occurrenceDateTime,
                           int occurrenceIndex) const;

    static QString frequencyToString(Frequency frequency);

    Frequency m_frequency;
    int m_interval;
    EndMode m_endMode;
    QDateTime m_untilDate;
    int m_maxOccurrences;
};

#endif