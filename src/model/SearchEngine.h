#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "Activity.h"

#include <QString>
#include <vector>

class SearchEngine
{
public:
    struct SearchResult
    {
        const Activity* activity = nullptr;
        int score = 0;
        QString matchedField;
    };

    struct SearchSuggestion
    {
        const Activity* activity = nullptr;
        QString suggestedText;
        QString matchedText;
        int distance = -1;

        bool isValid() const;
    };

    struct SearchResponse
    {
        std::vector<SearchResult> results;
        SearchSuggestion suggestion;

        bool hasResults() const;
    };

    static SearchResponse search(const std::vector<const Activity*>& activities,
                                 const QString& query);

private:
    static QString normalize(const QString& text);

    static int calculateFieldScore(const QString& fieldText,
                                   const QString& normalizedQuery,
                                   int exactScore,
                                   int startsWithScore,
                                   int containsScore);

    static int levenshteinDistance(const QString& first,
                                   const QString& second);

    static SearchSuggestion findBestSuggestion(const std::vector<const Activity*>& activities,
                                               const QString& normalizedQuery);

    static int maxAcceptedDistance(int queryLength);
};

#endif